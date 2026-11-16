# Stereo raw path cost (run 879)

**Verdict:** field stereo already shares matrix×vert, plane reject, face cull, zf/parallax, and y/z/uv/colour once (`G3G_TriClip2`). The remaining ~4–5 ms stereo–mono gap in `GpuRaw` is mostly **dual eye vertex/batch materialisation** (counted inside G3PROF `xform` on the miss path), plus a little edge-fix overdraw — not a second transform.

| Run / scene | stereo raw | of which “xform” | mono raw (expected) |
|---|---|---|---|
| 879 field (owner) | ~9.5 ms | ~3.8 ms | ~4–5 ms |
| 876 G3PROF field | ~8.2 ms | 2–7.5 ms | ~51 fps mono / ~44 stereo |
| #482 Goldenrod (older split) | raw ~7.0–7.5 | xform~3.1–3.5, queue~2.1 | mono/stereo raw similar when still |

Target: pull stereo raw toward mono-like **~5 ms** (−4 to −4.5 ms), without changing the picture when the new toggle is OFF.

---

## 1. Bit-identical per eye vs what must differ

### Already shared (one pass; same floats for both eyes)

| Work | Where |
|---|---|
| Clip matrix × model vert (16 MAD/vert) | `GpuRawJ` loop `g3soft.cpp:587-590` |
| Trivial reject + face cull | `:594-603` |
| zf + `parOf` / `lockSh` once; `shL=-sd*p`, `shR=+sd*p` | `:606-608` |
| y, z, w, s, t, rgba | `G3G_TriClip2` `g3gpu.c:539-544` (`q->* = o->*`) |
| Tex / wrap / blend / fog batch key | `batchFor` called for both eyes with the same args `:533` |

```605:609:soulsilver3ds/platform/g3soft.cpp
  if(!stereoSd){G3G_TriClip(0,c,su,sv,col,J.gt,tp.repeatS,tp.repeatT,tp.flipS,tp.flipT,translucent,dw,dtest,nullptr);continue;}
  /* 2026-11-04: zf and the parallax once per vertex, both eyes written in one G3G_TriClip2 pass (was: all of it per eye) */
  float zf3[3];bool degen=false;for(int k=0;k<3;k++){float w=c[4*k+3];zf3[k]=w>1e-6f?0.5f*c[4*k+2]/w+0.5f:1.0f;if(!degen&&zDegenerate(w,zf3[k]))degen=true;}if(degen)stereoClamped++;
  float shL[3],shR[3];for(int k=0;k<3;k++){if(parLock){shL[k]=lockSh(0,stereoSd);shR[k]=lockSh(1,stereoSd);}else{const float p=degen?0.65f:parOf(sprZf>=0.0f?sprZf:zf3[k]);shL[k]=-stereoSd*p;shR[k]=stereoSd*p;}}
  G3G_TriClip2(c,su,sv,col,J.gt,tp.repeatS,tp.repeatT,tp.flipS,tp.flipT,translucent,dw,dtest,shL,shR);
```

```528:546:soulsilver3ds/platform/g3gpu.c
/* 2026-11-04 ... only x differs between the eyes, but y, z, uv and colour are worked out once instead of twice. */
void G3G_TriClip2(...) {
    B *l0 = batchFor(0, ...), *l1 = batchFor(1, ...);
    ...
        o->x = x + shL[k] / 128.0f * w; q->x = x + shR[k] / 128.0f * w; if (wideFrame) { o->x *= 0.8f; q->x *= 0.8f; }
        o->y = ...; o->z = ...; o->w = w; ... colour ...
        q->y = o->y; q->z = o->z; q->w = w; q->s = o->s; q->t = o->t; q->r = o->r; q->g = o->g; q->b = o->b; q->a = o->a;
    l0->count += 3; l1->count += 3;
}
```

### Must differ (parallax / eye offset)

| Work | Why |
|---|---|
| `o->x` / `q->x` with `shL`/`shR` | Stereo disparity in clip x (`shift/128 * w`) |
| Separate `vb[0]` / `vb[1]`, `bt[0]` / `bt[1]` | `G3G_End` draws each eye into `tgt[e]` (`g3gpu.c:829`, `:937-984`) |
| Soft `Tri()` eye loop | CPU path still shifts screen x per eye (`g3soft.cpp:264-268`) |
| `qwK` widened by `stereoSd/128` when `stereo_edgefix=1` | Reject after shift would drop edge columns (`qwKNow` / edgeFix; `:571-574`) |

---

## 2. TriClip2 already shares xform — what ELSE doubles raw?

**Yes:** one matrix×vert + one reject/cull + one parallax, then one `TriClip2`. There is no second geometry transform for field raw stereo (HANDOFF: “Field stereo already one-xforms both eyes”).

What still costs ~2× vs mono inside **raw**:

1. **Dual `V` write + dual `batchFor`** (`g3gpu.c:533-546`).  
   `V` = 10 floats (40 B) × 3 verts × 2 eyes ≈ 240 B/tri. ~700–900 drawn tris → ~170–220 KiB written into linear `vb[]` every frame, plus batch bookkeeping. On ARM11 this dominates the stereo–mono raw delta.

2. **G3PROF bucket confusion (g3thr lineage):** on the xfcache **miss** path the whole triangle loop (including `TriClip2`) is charged to `prXform`; only the xfcache **hit** path splits `prQueue` for TriClip. So run 879 “xform 3.8” **includes** dual-eye queue, not pure MAD. Fix instrumentation first (below).

3. **`stereo_edgefix` wider x planes** (`:571-574`, `qwKNow`): a few more near-edge tris survive reject → slightly more TriClip2 work. Usually ≪1 ms.

4. **Hard-clip / screen path** (`:596-599` → `DrawScreenJ` → `Tri` eye loop `:264-268`): those tris still pay full dual eye setup. Usually a small fraction of field tris.

5. **`sprite_depth` stereo pre-pass** (`:576-584`): only small textured quads; noise-level.

6. **Not in raw:** `G3G_End` double draw/flush/readback (`:829`, eye loop `:937+`) — that is `end`, not `raw`.

Rough split of the **~4.5–5.5 ms** stereo−mono raw gap:

| Component | Expected |
|---|---|
| Dual vb/batch write (inside “xform” today) | **~3.0–4.0 ms** |
| Extra edge-fix tris + parallax math | ~0.2–0.5 ms |
| Hard-clip dual Tri | ~0.2–0.5 ms (scene-dependent) |
| True second matrix×vert | **~0** (already shared) |

---

## 3. Safe share opportunities (new toggle, default OFF)

Keep existing `g3_fastxform` / `g3_xfcache` / `g3_thread` as they are. Add one experimental Debug toggle:

### A. `g3_stereo_share` (recommended first) — default OFF

**Idea:** `TriClip2` writes **left only** (same as today’s `o->*`). At `G3G_End` (or a tiny expand just before flush), build right as:

- `memcpy` left `V` → right, then  
  `q->x = o->x + (shR−shL)/128*w`  
  with wide: apply the same `*0.8` already baked into `o->x` by expanding **before** wide, or store unscaled x / store `dx = (shR-shL)/128*w` in a parallel float array of size `vn[0]`.

Cleaner variant (bit-identical to today):

- While building left, also push `dx[k] = (shR[k]-shL[k])/128.f*w` (or `2*sd*p/128*w`) into a side buffer; expand: `q = *o; q.x = o->x + dx` (after wide multiply both the same way as now).

**Mirror batches:** after expand, `bt[1]` can be a structural copy of `bt[0]` (`first`/`count` identical when no hard-clip mixed in). Hard-clipped screen tris that already called `G3G_Tri` per eye stay on the old path when share is ON (or force those through expand too with per-eye screen shifts — denser; skip v1).

**OFF:** current `TriClip2` byte-for-byte.

**Expected saving:** **~2.5–4.0 ms** of stereo raw (most of the dual write). Ceiling ≈ mono raw + ~0.5 ms expand memcpy.

**Risk:** mid/fps60 `kr[].v0[1]`, reuse keys, and `vn[1]==vn[0]` assumptions; hard-clip mixes; `MAXV` still needs room for both eyes after expand.

### B. Already-landed levers (no new share code)

| Toggle | Effect on stereo raw | Expected |
|---|---|---|
| `g3_fastxform=1` | Hoist `M[]` locals; bit-identical MAD | **~0.2–0.8 ms** of true xform |
| `g3_xfcache=1` | Still camera → skip MAD, still pay TriClip2/queue | **0 walking**; **~1–3 ms** standing |
| `g3_thread` + `g3_thr_texfence` | Overlap front/cmd with raw (wall), not less raw CPU | wall frame, not raw ms |

### C. Later / higher risk (not v1)

- **Shader eye offset:** draw `vb[0]` twice with a uniform ±shift (like `g3k` MidK). Saves End flush of `vb[1]` and can drop right CPU expand. Touches PICA path + mid; separate PR.
- **Do not** re-run GpuRawJ per eye — already gone since 2026-11-04.

### Instrumentation prerequisite (same PR or tiny ahead-of)

On the miss path, time `G3G_TriClip2` into `prQueue` (as the hit path already does on g3thr). Then run 879-style logs will show true `xform` (MAD+reject) vs `queue` (dual write). Expect stereo `queue ≈ mono_queue×2` and stereo `xform ≈ mono_xform`.

---

## 4. How `g3_thr_check` / `xformCrc` / `snapCrc` verify a shared path

(On the g3thr / main lineage: `GJob.snapCrc` / `xformCrc`, Debug **3D thread check**, default OFF.)

| Check | What it hashes | Use for share |
|---|---|---|
| `snapCrc` (`gtSnapCrc`) | Job POD: `M`, `n`, `sd`, `parL`, mid/wide, cull/fog/tex, flags | Submit vs worker still must match with share ON (job inputs unchanged). |
| `xformCrc` (`gtXformCrc`) | First 48 verts × `M` → clip xyzw + colour crumbs + `sd`/`parL`/`wide`/tex size | Proves **clip transform** is bit-identical across thread/inline and across share ON/OFF (share must not change clip `c[]`). |
| Summary | Every 300f: `[G3THR] check: N jobs, M mismatches (want 0)` | Gate device A/B. |

**Extra verify for `g3_stereo_share` (add under `g3_thr_check=1` or `g3_stereo_share=2`):**

1. Shadow path: once per N frames, also run today’s dual `TriClip2` into a scratch `vbR_ref[]`.
2. Expand-from-left into `vb[1]`.
3. `memcmp` / FNV of `vb[1]` vs ref (and `bn`/`bt` structure). Log `[G3SHARE] eye mismatch` on first failures; auto-disable share like `fastBounds` does on mismatch.
4. Host: extend `tools/g3_thr_race_test.c` (or a small `g3_stereo_share_test.c`) with known verts → left+dx == dual TriClip2.

`xformCrc` alone does **not** cover right-eye `V` bytes; the eye memcmp above is required for share.

Device recipe:

```text
g3_prof=1 g3_thr_check=1 g3_stereo_share=0   # baseline: raw ~9.5, xform/queue split after instr
g3_stereo_share=1                            # expect raw → ~5–6 ms, check 0 mismatches, no eye mismatch
slider 0 vs full: mono raw unchanged with share OFF or ON
```

---

## 5. Concrete implementation plan (ordered)

1. **Measure (platform-only, ~20 LOC)**  
   - Split miss-path TriClip into `prQueue` in `GpuRawJ` (g3thr-style).  
   - Confirm stereo `queue ≈ 2× mono` and `xform` nearly equal.  
   - **Saving:** 0 ms (truth for the next step).

2. **`g3_stereo_share` default OFF**  
   - `g3gpu.c` `G3G_TriClip2`: if share ON, only `batchFor(0)` + write `vb[0]`; append `dx[]`.  
   - `G3G_End` start: expand `vb[1]`/`bt[1]` from left+dx (skip if `eyes<2`).  
   - Settings row + `kCfgDefaults` + logring `[G3SHARE]`.  
   - **Expected: −2.5 to −4.0 ms stereo raw** → **~5.5–7.0 ms** first cut.

3. **Verify**  
   - Wire eye CRC under `g3_thr_check` / share=2.  
   - Host unit test bit-identical to dual TriClip2.  
   - Goldenrod + Route 34 walk, wide on/off, battle `parLock`, mid/fps60 off first.

4. **Optional polish**  
   - Batch mirror without re-`batchFor(1)` during expand.  
   - `g3_fastxform=1` A/B on the now-honest `xform` bucket (−0.2–0.8 ms).  
   - Shader dual-draw PR later for End, not raw.

5. **Out of scope for this cut**  
   - Claiming 44→60 fps from share alone (front/cmd/end still dominate wall).  
   - Changing soft `Tri()` CPU stereo loop (rare in GPU field path).

---

## File:line index

| Item | Location |
|---|---|
| `GpuRawJ` transform + TriClip2 call | `soulsilver3ds/platform/g3soft.cpp:561-610` |
| Soft eye loop | `g3soft.cpp:264-268` |
| `stereoSd` / begin | `g3soft.cpp:199`, `:762-774` |
| `G3G_TriClip` / `TriClip2` | `soulsilver3ds/platform/g3gpu.c:522-557` |
| `batchFor` | `g3gpu.c:506-515` |
| `G3G_End` flush + eye draw | `g3gpu.c:824-984` |
| `g3_thr_check` / CRC (g3thr lineage) | `g3soft.cpp` `gtSnapCrc` / `gtXformCrc`; settings `g3_thr_check` |
| Prior stereo one-pass note | `HANDOFF.md` (solid-30 / g3_fastxform sections) |
