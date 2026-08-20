INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS tlp_span_in;

CREATE VIEW tlp_span_in
AS
WITH
  data AS (
    SELECT ts, LEAD(ts) OVER (ORDER BY ts ASC) - ts AS dur, active_cpu_count
    FROM sched_active_cpu_count
  )
SELECT * FROM data WHERE dur IS NOT NULL;

DROP TABLE IF EXISTS tlp_span;

CREATE VIRTUAL TABLE tlp_span
USING
  SPAN_JOIN(tlp_span_in, interesting_interval);

DROP VIEW IF EXISTS tlp_metric;

CREATE VIEW tlp_metric
AS
WITH
  collapse AS (
    SELECT
      interval_id,
      dur,
      IIF(active_cpu_count < 6, '' || active_cpu_count, '6+') AS tlp
    FROM tlp_span
  ),
  data AS (
    SELECT interval_id, SUM(dur) / 1e9 AS value, tlp FROM collapse GROUP BY interval_id, tlp
  )
SELECT interval_id, 'tlp_' || tlp AS metric_name, 'sec' AS unit, value FROM data;

SELECT * FROM tlp_metric;
