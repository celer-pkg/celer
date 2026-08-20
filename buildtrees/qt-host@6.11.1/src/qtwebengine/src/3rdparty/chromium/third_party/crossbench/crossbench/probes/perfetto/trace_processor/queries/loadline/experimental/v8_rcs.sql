INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS v8_rcs_metric;

CREATE VIEW v8_rcs_metric
AS
WITH
  dur AS (
    SELECT
      interval_id,
      replace(substr(key, 26), '[1]', '') AS name,
      sum(int_value * span_ratio) AS dur
    FROM interesting_slice_span, args
    USING (arg_set_id)
    WHERE key LIKE 'debug.runtime-call-stats%[1]'
    GROUP BY 1, 2
  ),
  count AS (
    SELECT
      interval_id,
      replace(substr(key, 26), '[0]', '') AS name,
      sum(int_value * span_ratio) AS count
    FROM interesting_slice_span, args
    USING (arg_set_id)
    WHERE key LIKE 'debug.runtime-call-stats%[0]'
    GROUP BY 1, 2
  ),
  data AS (
    SELECT
      interval_id,
      CASE
        WHEN name LIKE '%Total%' THEN 'total'
        WHEN name LIKE '%RegExp%' THEN 'regexp'
        WHEN name LIKE '%IC^_%' ESCAPE '^' THEN 'ic'
        WHEN name LIKE '%IC%Miss' THEN 'ic'
        WHEN name LIKE 'IC' THEN 'ic'
        WHEN name LIKE 'Json%' THEN 'json'
        WHEN name LIKE '%Optimize%Background%' THEN 'optimize_background'
        WHEN name LIKE '%Optimize%Concurrent%' THEN 'optimize_background'
        WHEN name LIKE 'StackGuard%' THEN 'optimize'
        WHEN name LIKE 'Optimize%' THEN 'optimize'
        WHEN name LIKE 'Deoptimize%' THEN 'optimize'
        WHEN name LIKE 'Recompile%' THEN 'optimize'
        WHEN name LIKE '%TierUp%' THEN 'optimize'
        WHEN name LIKE '%BudgetInterrupt%' THEN 'optimize'
        WHEN name LIKE 'Compile%Optimized%' THEN 'optimize'
        WHEN name LIKE '%Compile%Background%' THEN 'compile_background'
        WHEN name LIKE 'Compile%' THEN 'compile'
        WHEN name LIKE '%^_Compile%' ESCAPE '^' THEN 'compile'
        WHEN name LIKE '%CompileLazy%' THEN 'compile'
        WHEN name LIKE '%Parse%Background%' THEN 'parse_background'
        WHEN name LIKE 'Parse%' THEN 'parse'
        WHEN name LIKE 'PreParse%' THEN 'parse'
        WHEN name LIKE '%GetMoreDataCallback%' THEN 'network_data'
        WHEN name LIKE '%Callback%' THEN 'callback'
        WHEN name LIKE "%Blink C\+\+%" THEN 'callback'
        WHEN name LIKE '%API%' THEN 'api'
        WHEN name LIKE 'GC^_Custom^_%'  ESCAPE '^' THEN 'gc_custom'
        WHEN name LIKE 'GC^_%BACKGROUND%' ESCAPE '^' THEN 'gc_background'
        WHEN name LIKE 'GC^_%Background%' ESCAPE '^' THEN 'gc_background'
        WHEN name LIKE 'GC^_%AllocateInTargetSpace' ESCAPE '^' THEN 'gc'
        WHEN name LIKE 'GC_%' ESCAPE '^' THEN 'gc'
        WHEN name LIKE 'JS^_Execution' ESCAPE '^' THEN 'javascript'
        WHEN name LIKE 'JavaScript' THEN 'javascript'
        WHEN name LIKE '%Blink^_%' ESCAPE '^' THEN 'blink'
        ELSE 'runtime'
        END AS metric,
      SUM(dur) AS dur,
      SUM(count) AS count
    FROM dur, count
    USING (interval_id, name)
    GROUP BY interval_id, metric
  )
SELECT
  interval_id,
  'v8_rcs_' || metric || '_dur' AS metric_name,
  'sec' AS unit,
  dur / 1e6 AS value
FROM data
UNION ALL
SELECT
  interval_id,
  'v8_rcs_' || metric || '_count' AS metric_name,
  'count' AS unit,
  count AS value
FROM data;

SELECT * FROM v8_rcs_metric;
