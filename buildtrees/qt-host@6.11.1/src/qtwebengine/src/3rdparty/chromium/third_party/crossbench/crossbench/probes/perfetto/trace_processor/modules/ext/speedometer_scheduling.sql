-- TODO(carlscab): All this is still in experimental stage. Your use at your own
-- risk. We intend to move all these to the Perfetto stdlib once we have a
-- stable and convenient API
SELECT IMPORT('ext.chrome_tlp');
SELECT RUN_METRIC('chrome/chrome_processes.sql');

CREATE VIEW internal_ext_benchmark_scheduling_benchmark_slice
AS
SELECT iteration, suite_name, ts, dur FROM ext_benchmark_slice;

CREATE VIRTUAL TABLE internal_ext_benchmark_scheduling_sched_slice
USING
  SPAN_JOIN(internal_ext_benchmark_scheduling_benchmark_slice, sched_slice);

CREATE VIEW ext_benchmark_scheduling_thread_cpu_time
AS
WITH
  base AS (
    SELECT
      iteration,
      suite_name,
      utid,
      cpu,
      SUM(DUR) / 1e9 AS cpu_time
    FROM internal_ext_benchmark_scheduling_sched_slice
    WHERE utid <> 0 AND dur > 0
    GROUP BY 1, 2, 3
  ),
  chrome_thread_view AS (
    SELECT
      utid,
      1 AS is_chrome_thread,
      process_type AS chrome_process_type,
      canonical_name AS chrome_thread_name
    FROM
      chrome_process
    JOIN chrome_thread
      USING (upid)
  )
SELECT
  base.*,
  thread.name AS thread_name,
  process.name AS process_name,
  COALESCE(is_chrome_thread, 0) AS is_chrome_thread,
  chrome_process_type,
  chrome_thread_name
FROM
  base
JOIN thread
  USING (utid)
JOIN process
  USING (upid)
LEFT JOIN chrome_thread_view
  USING (utid);

CREATE VIRTUAL TABLE ext_benchmark_scheduling_tpl_slice
USING
  SPAN_JOIN(ext_benchmark_slice, ext_chrome_tlp_slice);

CREATE VIEW ext_benchmark_scheduling_tlp_by_test
AS
WITH
  base AS (
    SELECT
      iteration,
      suite_name,
      tlp * SUM(dur) AS weighted_tlp,
      chrome_tlp * SUM(dur) AS weighted_chrome_tlp,
      SUM(dur) AS total_cpu_time
    FROM ext_benchmark_scheduling_tpl_slice
    GROUP BY iteration, suite_name, tlp, chrome_tlp
  ),
  by_iter_test AS (
    SELECT
      iteration,
      suite_name,
      1.0 * SUM(weighted_tlp) / SUM(total_cpu_time) AS avg_tlp,
      1.0 * SUM(weighted_chrome_tlp) / SUM(total_cpu_time) AS avg_chrome_tlp
    FROM base
    GROUP BY iteration, suite_name
  ),
  median_base AS (
    SELECT
      suite_name,
      avg_tlp,
      avg_chrome_tlp,
      ROW_NUMBER()
        OVER (PARTITION BY suite_name ORDER BY avg_tlp ASC) n_avg_tlp,
      ROW_NUMBER()
        OVER (PARTITION BY suite_name ORDER BY avg_chrome_tlp ASC)
          n_avg_chrome_tlp,
      COUNT() OVER (PARTITION BY suite_name) n_total
    FROM by_iter_test
  ),
  tlp AS (
    SELECT
      suite_name,
      AVG(avg_tlp) AS median_avg_tlp
    FROM median_base
    WHERE (n_avg_tlp - 1) IN ((n_total / 2), ((n_total - 1) / 2))
    GROUP BY suite_name
  ),
  chrome_tlp AS (
    SELECT
      suite_name,
      AVG(avg_chrome_tlp) AS median_avg_chrome_tlp
    FROM median_base
    WHERE (n_avg_chrome_tlp - 1) IN (n_total / 2, (n_total - 1) / 2)
    GROUP BY suite_name
  )
SELECT * FROM tlp JOIN chrome_tlp USING (suite_name);
