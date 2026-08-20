-- TODO(carlscab): All this is still in experimental stage. Your use at your own
-- risk. We intend to move all these to the Perfetto stdlib once we have a
-- stable and convenient API
SELECT IMPORT('chrome.speedometer');

CREATE VIEW internal_ext_benchmark_slice_mark
AS
WITH
  timing_slices AS (
    SELECT *
    FROM slice
    WHERE category = 'blink.user_timing'
  )
    SELECT
      track_id,
      ts,
      substr(name, 28) AS suite_name,
      1 AS is_start
    FROM timing_slices
    WHERE name GLOB 'benchmark-test_suite-start-*'
    UNION ALL
    SELECT
      track_id,
      ts,
      substr(name, 26) AS suite_name,
      0 AS is_start
    FROM timing_slices
    WHERE name GLOB 'benchmark-test_suite-end-*';

CREATE VIEW internal_ext_benchmark_slice
AS
WITH
  next AS (
    SELECT
      track_id,
      ts,
      suite_name,
      is_start,
      LEAD(suite_name) OVER (win) AS next_suite_name,
      LEAD(is_start) OVER (win) AS next_is_start,
      LEAD(ts) OVER (win) AS next_ts
    FROM internal_ext_benchmark_slice_mark
    WINDOW win AS (PARTITION BY track_id ORDER BY ts ASC)
  )
SELECT
  ts,
  CAST(next_ts - ts AS INT) AS dur,
  ROW_NUMBER()
    OVER (PARTITION BY track_id, suite_name ORDER BY ts ASC) AS iteration,
  suite_name
FROM next
WHERE is_start AND NOT next_is_start AND suite_name = next_suite_name;

CREATE TABLE ext_benchmark_slice
AS
WITH
  speedometer AS (
    SELECT
      iteration,
      suite_name,
      test_name,
      measure_type AS measure,
      ts,
      dur
    FROM
      chrome_speedometer_measure
  ),
  other AS (
    SELECT
      iteration,
      suite_name,
      suite_name AS test_name,
      suite_name AS measure,
      ts,
      dur
    FROM
      internal_ext_benchmark_slice
  ),
  merged AS (
    -- TODO: Do something if we find rows for both tables (eg. take only speedometer ones?)
    SELECT * FROM other
    UNION ALL
    SELECT * FROM speedometer
  )
SELECT
  ROW_NUMBER() OVER () AS benchmark_slice_id,
  iteration,
  suite_name,
  test_name,
  measure,
  ts,
  dur
FROM merged;

CREATE
  UNIQUE INDEX ext_benchmark_slice_indx
ON ext_benchmark_slice(benchmark_slice_id);

SELECT
  CREATE_FUNCTION(
    'EXT_BENCHMARK_RENDER_MAIN_UTID()',
    'INT',
    "
      WITH
      union_track AS (
        SELECT track_id
        FROM slice
        WHERE id IN (SELECT slice_id FROM chrome_speedometer_measure)
        UNION ALL
        SELECT track_id
        FROM internal_ext_benchmark_slice_mark
      ), benchmark_track AS (
        SELECT DISTINCT track_id
        FROM union_track
      )
    SELECT utid
    FROM thread_track
    WHERE
      id IN (SELECT * FROM benchmark_track)
        ");
