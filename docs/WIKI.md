# Project wiki

The [complete GitHub wiki](https://github.com/unowncollectables/Pokemon-HeartGold-SoulSilver-New3ds/wiki) contains player guides, all tracked repository Markdown documentation, the full settings text reference, tool guides, community policies, the license, and historical archives.

Current player guidance is separated from historical engineering notes. Archive pages carry a warning; mixed-age settings, wireless, and agent references point back to the current guides. Each imported page links to its exact source revision, including images and source-file links.

## Update it from main

Use an up-to-date main checkout. Review the player summaries in `scripts/build_wiki.py` against the newest README, FAQ, handoff, and release notes before publishing. Importing a newer reference page does not automatically rewrite those summaries.

```sh
git clone https://github.com/unowncollectables/Pokemon-HeartGold-SoulSilver-New3ds.wiki.git ../hgss-wiki
python3 scripts/build_wiki.py ../hgss-wiki
git -C ../hgss-wiki diff --stat
git -C ../hgss-wiki add -- '*.md'
git -C ../hgss-wiki commit -m "Refresh complete project wiki"
git -C ../hgss-wiki push origin HEAD
```

Pull first when reusing an existing wiki clone. The exporter validates relative wiki page links and rejects an output directory inside the source checkout. It does not push, delete pages, or access game data. Existing generated pages are overwritten; make lasting edits in the source document or exporter. Removed or renamed source documents may leave old wiki pages, so review those explicitly.

GitHub requires an initialized wiki and normal Git push access. No game build or separate automation token is needed. Wiki publication is a separate Git commit from the source repository PR.
