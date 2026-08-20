INCLUDE PERFETTO MODULE chrome.page_loads;
INCLUDE PERFETTO MODULE sched.thread_level_parallelism;

DROP VIEW IF EXISTS interesting_interval_with_overlaps;

CREATE VIEW interesting_interval_with_overlaps
AS
WITH
  offset(i, len) AS (VALUES(0, 1), (1, 2), (2, 3), (3, 5)),
  INTERVAL AS (
    SELECT
      id AS page_load_id,
      navigation_start_ts AS ts,
      navigation_start_ts,
      0 AS lcp_offset,
      lcp AS dur,
      0 AS i,
      0 AS len
    FROM chrome_page_loads
    WHERE lcp IS NOT NULL
    UNION ALL
    SELECT
      page_load_id,
      ts + dur AS ts,
      navigation_start_ts,
      lcp_offset + offset.len AS lcp_offset,
      offset.len * 1000 * 1000 * 1000 AS dur,
      i + 1,
      offset.len
    FROM INTERVAL, offset
    USING (i)
  ),
  data AS (
    SELECT
      ROW_NUMBER() OVER (ORDER BY page_load_id, len) AS interval_id,
      *,
      ts + dur AS END
    FROM INTERVAL
  ),
  overlap AS (
    SELECT
      *,
      (
        SELECT MIN(other.interval_id)
        FROM data AS other
        WHERE
          interval.interval_id
            <> other.interval_id
            AND other.ts < interval.end
            AND interval.ts < other.end
      ) AS first_overlap_interval_id
    FROM data AS INTERVAL
    ORDER BY 1, 2 ASC
  )
SELECT
  interval_id,
  page_load_id,
  IIF(len <> 0, PRINTF('lcp+%02d-lcp+%02d', lcp_offset - len, lcp_offset), 'NAV_START-lcp')
    AS interval_name,
  first_overlap_interval_id,
  ts,
  navigation_start_ts,
  dur,
    END
FROM overlap;

DROP VIEW IF EXISTS interesting_interval;

CREATE VIEW interesting_interval
AS
SELECT
  *
FROM interesting_interval_with_overlaps
WHERE page_load_id = (SELECT MAX(page_load_id) FROM interesting_interval_with_overlaps);

DROP VIEW IF EXISTS interesting_slice_span_in;

CREATE VIEW interesting_slice_span_in
AS
SELECT *, dur AS original_dur
FROM slice;

DROP TABLE IF EXISTS interesting_slice_span_internal;

CREATE VIRTUAL TABLE interesting_slice_span_internal
USING
  SPAN_JOIN(interesting_slice_span_in PARTITIONED track_id, interesting_interval);

DROP VIEW IF EXISTS interesting_slice_span;

CREATE VIEW interesting_slice_span
AS
SELECT
  interval_id,
  ts,
  dur AS span_dur,
  original_dur,
  name,
  arg_set_id,
  IIF(original_dur = 0, 1, 1.0 * dur / original_dur) AS span_ratio,
  IIF(original_dur = 0, 0.0, 1.0 * thread_dur * dur / original_dur) AS thread_span_dur,
  track_id
FROM interesting_slice_span_internal;

DROP VIEW IF EXISTS interesting_slice_start_in;

CREATE VIEW interesting_slice_start_in
AS
SELECT ts, 0 AS dur, dur AS original_dur, name, arg_set_id, track_id
FROM slice;

DROP TABLE IF EXISTS interesting_slice_start_span;

CREATE VIRTUAL TABLE interesting_slice_start_span
USING
  SPAN_JOIN(interesting_slice_start_in, interesting_interval);

DROP VIEW IF EXISTS interesting_slice_start;

CREATE VIEW interesting_slice_start
AS
SELECT interval_id, ts, original_dur, name, arg_set_id, track_id
FROM interesting_slice_start_span;

DROP VIEW IF EXISTS interesting_slice_end_in;

CREATE VIEW interesting_slice_end_in
AS
SELECT ts + dur AS ts, 0 AS dur, ts AS original_ts, dur AS original_dur, name, arg_set_id, track_id
FROM slice
ORDER BY ts ASC;

DROP TABLE IF EXISTS interesting_slice_end_span;

CREATE VIRTUAL TABLE interesting_slice_end_span
USING
  SPAN_JOIN(interesting_slice_end_in, interesting_interval);

DROP VIEW IF EXISTS interesting_slice_end;

CREATE VIEW interesting_slice_end
AS
SELECT interval_id, ts AS end_ts, original_ts, original_dur, name, arg_set_id, track_id
FROM interesting_slice_end_span;
