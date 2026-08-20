-- Trace categories needed:
--   * benchmark
--   * blink.user_timing
--   * loading
--   * devtools.timeline
--   * disabled-by-default-devtools.timeline
--   * v8

INCLUDE PERFETTO MODULE slices.with_context;

CREATE OR REPLACE PERFETTO FUNCTION get_event_time(name STRING)
RETURNS INT
AS
SELECT ts
FROM slice
WHERE name GLOB $name AND category = 'blink.user_timing'
ORDER BY ts
LIMIT 1;

CREATE OR REPLACE PERFETTO FUNCTION get_next_presentation_time(ts INT)
RETURNS INT
AS
WITH
  candidate_presentation_time AS (
    SELECT a.ts + a.dur AS ts
    FROM slice s, ancestor_slice(s.id) a
    WHERE
      s.name = 'Commit'
      AND a.name = 'PipelineReporter'
      AND s.depth - 1 = a.depth
      AND s.ts > $ts
    ORDER BY s.ts
    LIMIT 1
  )
SELECT ts
FROM slice
WHERE
  name = 'Display::FrameDisplayed'
  AND ts >= (SELECT ts FROM candidate_presentation_time)
ORDER BY ts
LIMIT 1;

-- Use get_presentation_time() instead. See the comment in that function.
CREATE OR REPLACE PERFETTO FUNCTION get_first_presentation_time_for_event(
  name STRING)
RETURNS INT
AS
SELECT get_next_presentation_time(get_event_time($name));

CREATE OR REPLACE PERFETTO FUNCTION get_next_presentation_time_by_pid(
    ts INT, pid INT)
RETURNS INT
AS
SELECT MIN(a.ts + a.dur) AS ts
FROM process_slice s, ancestor_slice(s.id) a
WHERE
  s.name = 'Commit'
  AND a.name = 'PipelineReporter'
  AND s.depth - 1 = a.depth
  AND s.ts > $ts
  AND s.pid = $pid
  -- TODO(crbug.com/409484302): Once we are no longer interested in Chrome
  -- versions <=M136, leave only 'frame_reporter'.
  AND COALESCE(
        EXTRACT_ARG(a.arg_set_id, 'frame_reporter.state'),
        EXTRACT_ARG(a.arg_set_id, 'chrome_frame_reporter.state')
      ) = 'STATE_PRESENTED_ALL';

-- Gets the timestamp of the presentation of the frame that was committed after
-- the given performance.mark() event. This should theoretically represent the
-- time when changes to the DOM become visible on the screen.
-- This is an improved version of get_first_presentation_time_for_event(),
-- used for Loadline 1. The original method doesn't work for Loadline 2+ because
-- it doesn't account for multiple renderer processes in the trace.
-- Loadline 1 was kept using the old method to avoid accidental score changes.
CREATE OR REPLACE PERFETTO FUNCTION get_presentation_time(name STRING)
RETURNS INT
AS
WITH
  event AS (
    SELECT ts, pid
    FROM thread_slice
    WHERE name GLOB $name AND category = 'blink.user_timing'
    ORDER BY ts
    LIMIT 1
  )
SELECT get_next_presentation_time_by_pid(ts, pid)
FROM event;
