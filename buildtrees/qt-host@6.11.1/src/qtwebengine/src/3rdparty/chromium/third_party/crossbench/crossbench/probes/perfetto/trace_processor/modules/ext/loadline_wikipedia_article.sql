INCLUDE PERFETTO MODULE ext.first_presentation_time;
INCLUDE PERFETTO MODULE ext.navigation_start;

-- This metric returns the timestamp of the last important event (including
-- image paint, JS script runs etc.) since the beginning of the page load.
CREATE OR REPLACE PERFETTO FUNCTION loadline_wikipedia_article_score()
RETURNS FLOAT
AS
WITH
  script_run AS (
    SELECT MAX(ts + dur) AS script_run
    FROM slice
    WHERE
      name = 'v8.run'
      AND EXTRACT_ARG(arg_set_id, 'debug.fileName')
        GLOB '*ext.cx.entrypoints.languagesearcher.init*'
  ),
  img_load AS (
    SELECT ts
    FROM slice
    WHERE
      name = 'PaintImage'
      AND EXTRACT_ARG(arg_set_id, 'debug.data.url')
        GLOB '*Taylor_Swift_at_the_2023_MTV_Video_Music_Awards*'
  ),
  img_next_af AS (
    SELECT id
    FROM slice, img_load
    WHERE
      name = 'AnimationFrame'
      AND slice.ts > img_load.ts
    ORDER BY slice.ts
    LIMIT 1
  ),
  img_presentation AS (
    SELECT MAX(ts) AS img_presentation
    FROM img_next_af, DIRECTLY_CONNECTED_FLOW(img_next_af.id) AS flow, slice
    WHERE slice.id = flow.slice_in
  ),
  last_event AS (
    SELECT
      MAX(
        (SELECT * FROM img_presentation), (SELECT * FROM script_run))
  )
SELECT
  -- Multiply by 60 to make the score per minutes rather than per second.
  60e9 / (
    (SELECT * FROM last_event) - first_navigation_start());
