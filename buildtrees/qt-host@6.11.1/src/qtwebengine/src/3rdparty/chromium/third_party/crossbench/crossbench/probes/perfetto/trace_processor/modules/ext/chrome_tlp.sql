-- TODO(carlscab): All this is still in experimental stage. Your use at your own
-- risk. We intend to move all these to the Perfetto stdlib once we have a
-- stable and convenient API
SELECT RUN_METRIC('chrome/chrome_processes.sql');

CREATE VIEW ext_chrome_tlp_slice
AS
WITH
  chrome_thread_view AS (
    SELECT
      utid,
      1 AS is_chrome_thread
    FROM
      chrome_process
    JOIN chrome_thread
      USING (upid)
  ),
  base AS (
    SELECT
      SCHED_SLICE.*,
      COALESCE(is_chrome_thread, 0) AS is_chrome_thread
    FROM SCHED_SLICE
    LEFT JOIN chrome_thread_view
      USING (utid)
    WHERE dur > 0 AND utid <> 0
  ),
  events AS (
    SELECT ts, 1 AS tlp, IIF(is_chrome_thread, 1, 0) AS chrome_tlp FROM base
    UNION ALL
    SELECT ts + dur AS ts, -1 AS tlp, IIF(is_chrome_thread, -1, 0) AS chrome_tlp
    FROM base
  ),
  cum AS (
    SELECT
      ts,
      LEAD(ts) OVER (win) - ts AS dur,
      SUM(tlp) OVER (win) AS tlp,
      SUM(chrome_tlp) OVER (win) AS chrome_tlp
    FROM events
    WINDOW win AS (ORDER BY ts ASC)
  )
SELECT ts, dur, tlp, chrome_tlp
FROM cum
WHERE dur IS NOT NULL AND dur > 0;
