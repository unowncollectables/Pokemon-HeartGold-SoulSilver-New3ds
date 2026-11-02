/* online trading M4 (docs/WIRELESS.md Option B; GTS first): 3DS sockets for the game's own Wi-Fi Connection code.
 * pspoke already links cybervisi0n's libntrwifi (CPS TCP/IP) and libntrdwc (login, GTS); patch 0137 builds them with the
 * Linux socket backend (real BSD sockets plus a select thread). That backend needs three things from the platform:
 *   1. sockets: socInit with a 0x100000 byte aligned buffer (P3DS_NetInit, called from wm_uds.c when wireless=2),
 *   2. the local IP: getifaddrs() below (the 3DS's own Wi-Fi connection, so WPA2 works; the IP is read by connecting a UDP
 *      socket towards the DNS server and asking getsockname, which needs no extra libctru calls),
 *   3. the DNS server: the community Wi-Fi Connection DNS answers nas/gamestats2/gpcm with the revival servers' addresses,
 *      so CPSDnsIp (host-order IPv4, like CPSMyIp) is set to it. Default 178.62.43.212 (Poke Classic Network);
 *      config.txt key wifi_dns=<ipv4> overrides.
 * 4. the console identity: the game starts the connection only when DWC_CheckValidConsole() says the save's Wi-Fi profile
 *      belongs to this console. A real DS keeps the attested Wi-Fi ID in flash; the port has none, so (online mode only)
 *      the port adopts the user id already stored in the save (read only, nothing is written to the save), see
 *      __wrap_DWC_CheckValidConsole / __wrap_DWC_Auth_GetId (link wraps, soulsilver3ds/full/Makefile).
 * 5. the cheat gate: online mode is refused while any cheat_* option is on (no sockets, no identity, no traffic).
 * Nothing here touches the save. Log tag [NET]; every step is flushed to the SD card at once. */
#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern void PSPNativeMemLog(const char *, ...);
extern void P3DS_LogFlushSync(void);
extern int P3DS_CfgStr(const char *, char *, int);
extern int P3DS_CfgInt(const char *, int);
extern int P3DS_Cheat(int);        /* render3ds.cpp: cheat_money .. cheat_candy, k = 0..5 */
extern int P3DS_CheatNoEncCfg(void);
extern unsigned long CPSDnsIp[2]; /* libntrwifi cps; u32 there, 4 bytes on this ABI */
#define NLOG(...) do { PSPNativeMemLog(__VA_ARGS__); P3DS_LogFlushSync(); } while (0)

#define DEFAULT_DNS "178.62.43.212"
#define SOC_BUF_SIZE 0x100000

static int sState; /* 0 untried, 1 sockets up, -1 failed */
static void *sSocBuf;

static void netExit(void)
{
    if (sState == 1) socExit();
    free(sSocBuf);
    sSocBuf = NULL;
    sState = 0;
}

/* nonzero when a cheat is on; the first refusal is logged once per run */
int P3DS_OnlineCheatBlocked(void)
{
    static int told;
    int on = P3DS_CheatNoEncCfg() || P3DS_CfgInt("cheat_time", 0) != 0;
    for (int k = 0; k < 6; k++) on |= P3DS_Cheat(k) != 0;
    if (on && !told) {
        told = 1;
        NLOG("[NET] online mode refused: a cheat_* option is on (cheat_money/catch/shiny/eggs/exp/candy, cheat_no_encounters, cheat_time). No sockets, no identity, no traffic");
    }
    return on;
}

int P3DS_NetInit(void)
{
    if (sState) return sState > 0;
    if (P3DS_OnlineCheatBlocked()) return 0; /* sState stays 0: re-checked next call, so turning the cheat off works after a restart of the game */
    sState = -1;
    u32 wifi = 0;
    Result rc = acInit();
    if (R_SUCCEEDED(rc)) {
        rc = ACU_GetWifiStatus(&wifi);
        acExit();
    }
    NLOG("[NET] Wi-Fi status rc=0x%08lx status=%lu", (unsigned long)rc, (unsigned long)wifi);
    if (R_FAILED(rc) || wifi == 0) {
        NLOG("[NET] no Wi-Fi connection: online mode stays off");
        return 0;
    }
    sSocBuf = memalign(0x1000, SOC_BUF_SIZE);
    rc = sSocBuf ? socInit(sSocBuf, SOC_BUF_SIZE) : -1;
    NLOG("[NET] socInit rc=0x%08lx", (unsigned long)rc);
    if (R_FAILED(rc)) {
        free(sSocBuf);
        sSocBuf = NULL;
        return 0;
    }
    sState = 1;
    atexit(netExit);
    return 1;
}

/* struct layout must match toolchain-3ds/pspshim/ifaddrs.h (network.c is compiled with that header) */
struct ifaddrs {
    struct ifaddrs *ifa_next;
    char *ifa_name;
    unsigned int ifa_flags;
    struct sockaddr *ifa_addr;
    struct sockaddr *ifa_netmask;
};

static in_addr_t gDns; /* network order */

static void setDns(void)
{
    char dns[64];
    if (!P3DS_CfgStr("wifi_dns", dns, sizeof dns) || !dns[0]) strcpy(dns, DEFAULT_DNS);
    in_addr_t a = inet_addr(dns);
    if (a == (in_addr_t)-1) {
        NLOG("[NET] wifi_dns=%s is not an IPv4 address, using %s", dns, DEFAULT_DNS);
        a = inet_addr(DEFAULT_DNS);
    }
    gDns = a;
    CPSDnsIp[0] = CPSDnsIp[1] = ntohl(a);
    NLOG("[NET] DNS server %s", dns);
}

int getifaddrs(struct ifaddrs **ifap)
{
    static struct ifaddrs ifa;
    static struct sockaddr_in sa;
    static char name[] = "wlan0";
    *ifap = NULL;
    if (!P3DS_NetInit()) return 0;
    setDns();
    in_addr_t ip = 0;
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd >= 0) {
        struct sockaddr_in to, me;
        socklen_t len = sizeof me;
        memset(&to, 0, sizeof to);
        to.sin_family = AF_INET;
        to.sin_port = htons(53);
        to.sin_addr.s_addr = gDns;
        if (connect(fd, (struct sockaddr *)&to, sizeof to) == 0 && getsockname(fd, (struct sockaddr *)&me, &len) == 0) ip = me.sin_addr.s_addr;
        close(fd);
    }
    NLOG("[NET] local IP %u.%u.%u.%u", (unsigned)(ip & 255), (unsigned)((ip >> 8) & 255), (unsigned)((ip >> 16) & 255), (unsigned)(ip >> 24));
    if (!ip) return 0;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = ip;
    ifa.ifa_next = NULL;
    ifa.ifa_name = name;
    ifa.ifa_addr = (struct sockaddr *)&sa;
    *ifap = &ifa;
    return 0;
}

void freeifaddrs(struct ifaddrs *list) { (void)list; }

/* ---- console identity (link wraps) ---------------------------------------------------------------------------------
 * DWCAccUserData (libntrdwc include/base/dwc_account.h): int size; DWCAccLoginId pseudo; DWCAccLoginId authentic; int gs_profile_id;
 * each DWCAccLoginId is 12 bytes, so `authentic` is at offset 16. DWCAuthWiFiId is { u64 uId; u64 notAttestedId; u32 flg; }. */
extern unsigned long long DWCi_Acc_GetUserId(const void *loginId);
extern int __real_DWC_CheckHasProfile(const void *userData);
extern int __real_DWC_CheckValidConsole(const void *userData);
extern void __real_DWC_Auth_GetId(void *id);
struct AuthWiFiId { unsigned long long uId; unsigned long long notAttestedId; unsigned int flg; };
static unsigned long long sAdoptedUid;
static int sAdopted;

static int onlineMode(void) { return P3DS_CfgInt("wireless", 0) == 2; }

int __wrap_DWC_CheckValidConsole(const void *userData)
{
    static int told;
    if (userData && onlineMode() && !P3DS_OnlineCheatBlocked() && P3DS_NetInit() && __real_DWC_CheckHasProfile(userData)) {
        unsigned long long uid = DWCi_Acc_GetUserId((const char *)userData + 16);
        if (!sAdopted || sAdoptedUid != uid) {
            sAdopted = 1;
            sAdoptedUid = uid;
            NLOG("[NET] adopting the save's console id %08lx%08lx (the save has a Wi-Fi profile): the connection task may run", (unsigned long)(uid >> 32), (unsigned long)uid);
        }
        return 1;
    }
    if (!told) {
        told = 1;
        NLOG("[NET] console check: real answer (online %s)", onlineMode() ? "on, but not usable (cheat, no sockets or no profile)" : "off");
    }
    return __real_DWC_CheckValidConsole(userData);
}

void __wrap_DWC_Auth_GetId(void *id)
{
    __real_DWC_Auth_GetId(id);
    if (sAdopted && onlineMode()) {
        struct AuthWiFiId *a = id;
        a->uId = sAdoptedUid;
        a->flg = 1;
    }
}

/* The save needs a GameSpy profile (gs_profile_id > 0) before the game runs its connection task; otherwise it launches the
 * Wi-Fi setup app (untranslated) instead. A real DS creates the profile on its first login. Online mode lets the task run
 * without one; the DWC login then creates the account (the profile lands in the save only if that succeeds). */
int __wrap_DWC_CheckHasProfile(const void *userData)
{
    static int told;
    int real = __real_DWC_CheckHasProfile(userData);
    if (!told) {
        told = 1;
        NLOG("[NET] save has a Wi-Fi profile: %s%s", real ? "yes" : "no", (!real && onlineMode() && !P3DS_OnlineCheatBlocked()) ? " (online mode: connection task allowed anyway, the login will create one)" : "");
    }
    if (!real && onlineMode() && !P3DS_OnlineCheatBlocked() && P3DS_NetInit()) return 1;
    return real;
}

/* ---- run 83: the game's ghttp (GameSpy HTTP) for the GTS login --------------------------------------------------------------
 * After the netcheck the GTS fetches http://gamestats2.gs.nintendowifi.net/pokemondpds/worldexchange/info.asp?pid=..  (a 32 byte
 * challenge) and then a second URL with a SHA1 of it (a 22 byte answer). The game starts a request with ov00_021FA21C(url, cb, ud)
 * and polls ov00_021FA118() every frame; cb(buf, len, result, ud) is called from that poll (result 0 = ok).
 * Here: a plain HTTP/1.0 GET over the same soc sockets, run in a worker thread (the game thread never blocks); the name is resolved
 * by asking the community DNS (wifi_dns, default as above) directly over UDP, because the system DNS knows nothing of these hosts.
 * Online mode only, never with a cheat on. Log tag [GHTTP]. Plain GET only, nothing is POSTed, nothing touches the save. */
#include <poll.h>
#include <errno.h>
#include <fcntl.h>

static in_addr_t ghDnsAddr(void)
{
    if (!gDns) setDns();
    return gDns;
}

static int ghResolve(const char *host, in_addr_t *out)
{
    unsigned char q[300], r[600];
    int n = 0, fd;
    memset(q, 0, 12); q[0] = 0x53; q[1] = 0x31; q[2] = 1; q[5] = 1; n = 12; /* id, RD, 1 question */
    for (const char *p = host; *p;) {
        const char *d = strchr(p, '.'); int l = d ? (int)(d - p) : (int)strlen(p);
        if (l <= 0 || l > 63 || n + l + 6 > (int)sizeof q) return -1;
        q[n++] = (unsigned char)l; memcpy(q + n, p, l); n += l; p += l; if (d) p++;
    }
    q[n++] = 0; q[n++] = 0; q[n++] = 1; q[n++] = 0; q[n++] = 1; /* type A, class IN */
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) return -2;
    struct sockaddr_in to; memset(&to, 0, sizeof to); to.sin_family = AF_INET; to.sin_port = htons(53); to.sin_addr.s_addr = ghDnsAddr();
    int got = -3;
    for (int tries = 0; tries < 3 && got < 0; tries++) {
        if (sendto(fd, q, n, 0, (struct sockaddr *)&to, sizeof to) < 0) { got = -4; break; }
        struct pollfd pf = { fd, POLLIN, 0 };
        if (poll(&pf, 1, 2000) > 0) { int m = recvfrom(fd, r, sizeof r, 0, NULL, NULL); if (m > 12) got = m; }
    }
    close(fd);
    if (got < 12) return -5;
    int an = (r[6] << 8) | r[7], pos = 12;
    while (pos < got && r[pos]) pos += (r[pos] >= 0xC0) ? 2 : r[pos] + 1; /* question name */
    pos += 5;
    for (int i = 0; i < an && pos + 12 <= got; i++) {
        while (pos < got && r[pos] && !(r[pos] >= 0xC0)) pos += r[pos] + 1;
        pos += (pos < got && r[pos] >= 0xC0) ? 2 : 1;
        if (pos + 10 > got) break;
        int type = (r[pos] << 8) | r[pos + 1], rl = (r[pos + 8] << 8) | r[pos + 9]; pos += 10;
        if (type == 1 && rl == 4 && pos + 4 <= got) { memcpy(out, r + pos, 4); return 0; }
        pos += rl;
    }
    return -6;
}

#define GH_MAX 1024
static struct { volatile int state; /* 0 idle, 1 running, 2 done */ char url[400]; char body[GH_MAX]; int len, result; void *cb, *ud; Thread th; } gh;

static void ghWork(void *arg)
{
    (void)arg;
    char host[128], path[300]; const char *u = gh.url; int result = 5, len = 0;
    if (strncmp(u, "http://", 7) == 0) u += 7;
    const char *sl = strchr(u, '/');
    int hl = sl ? (int)(sl - u) : (int)strlen(u);
    if (hl > 0 && hl < (int)sizeof host) {
        memcpy(host, u, hl); host[hl] = 0; snprintf(path, sizeof path, "%s", sl ? sl : "/");
        in_addr_t ip = 0; int rr = ghResolve(host, &ip);
        NLOG("[GHTTP] %s -> %s (%s)", host, rr == 0 ? inet_ntoa(*(struct in_addr *)&ip) : "lookup failed", rr == 0 ? "ok" : "err");
        if (rr == 0) {
            int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            struct sockaddr_in sa; memset(&sa, 0, sizeof sa); sa.sin_family = AF_INET; sa.sin_port = htons(80); sa.sin_addr.s_addr = ip;
            if (fd >= 0 && connect(fd, (struct sockaddr *)&sa, sizeof sa) == 0) {
                char req[600]; int rl = snprintf(req, sizeof req, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: GameSpyHTTP/1.0\r\nConnection: close\r\n\r\n", path, host);
                char resp[2048]; int tot = 0;
                if (send(fd, req, rl, 0) == rl) {
                    for (;;) {
                        struct pollfd pf = { fd, POLLIN, 0 };
                        if (poll(&pf, 1, 6000) <= 0) break;
                        int m = recv(fd, resp + tot, (int)sizeof resp - 1 - tot, 0);
                        if (m <= 0) break;
                        tot += m; if (tot >= (int)sizeof resp - 1) break;
                    }
                }
                resp[tot] = 0;
                NLOG("[GHTTP] got %d bytes, status line: %.40s", tot, tot > 0 ? resp : "(none)");
                char *body = strstr(resp, "\r\n\r\n");
                if (tot > 12 && strncmp(resp, "HTTP/", 5) == 0 && strstr(resp, " 200") && strstr(resp, " 200") < resp + 16 && body) {
                    body += 4; len = tot - (int)(body - resp); if (len > GH_MAX - 1) len = GH_MAX - 1;
                    memcpy(gh.body, body, len); gh.body[len] = 0; result = 0;
                    NLOG("[GHTTP] ok, body %d bytes", len);
                }
            } else NLOG("[GHTTP] connect failed (errno %d)", errno);
            if (fd >= 0) close(fd);
        }
    }
    gh.len = len; gh.result = result; gh.state = 2;
}

/* returns a request id (>= 0) or -5 when it cannot start */
int P3DS_GhttpStart(const char *url, void *cb, void *ud)
{
    if (!url || !onlineMode() || P3DS_OnlineCheatBlocked() || !P3DS_NetInit()) { NLOG("[GHTTP] not started (online mode off, cheat on, or sockets down): %s", url ? url : "(null)"); return -5; }
    if (gh.state != 0) { NLOG("[GHTTP] busy, refused: %s", url); return -5; }
    snprintf(gh.url, sizeof gh.url, "%s", url); gh.cb = cb; gh.ud = ud; gh.len = 0; gh.result = 5; gh.state = 1;
    NLOG("[GHTTP] GET %s", gh.url);
    gh.th = threadCreate(ghWork, NULL, 0x8000, 0x30, -2, false);
    if (!gh.th) { gh.state = 0; NLOG("[GHTTP] thread create failed"); return -5; }
    return 1;
}

/* per-frame: finish a request on the game thread (the callback is game code) */
int P3DS_GhttpThink(void)
{
    if (gh.state == 2) {
        int (*cb)(char *, int, int, void *) = (int (*)(char *, int, int, void *))gh.cb;
        void *ud = gh.ud; int len = gh.len, res = gh.result;
        threadJoin(gh.th, 2000000000ULL); threadFree(gh.th);
        gh.th = NULL; gh.state = 0; /* idle first: the callback starts the next request */
        NLOG("[GHTTP] callback result %d len %d", res, len);
        if (cb) cb(res == 0 ? gh.body : NULL, len, res, ud);
    }
    return 1;
}
