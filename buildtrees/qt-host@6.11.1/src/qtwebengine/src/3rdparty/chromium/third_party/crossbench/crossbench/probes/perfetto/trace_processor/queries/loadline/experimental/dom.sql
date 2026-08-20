INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS dom_metric;

CREATE VIEW dom_metric
AS
WITH
  data AS (
    SELECT
      interval_id,
      'dom_' || substr(key, 12) AS metric_name,
      MAX(coalesce(int_value, real_value)) AS value
    FROM interesting_slice_span, args
    USING (arg_Set_id)
    WHERE name = 'UpdateCounters'
    GROUP BY 1, 2, track_id
  )
SELECT
  interval_id,
  metric_name,
  CASE
    WHEN metric_name LIKE "%HeapSize%" THEN 'bytes'
    ELSE 'count'
    END AS unit,
  sum(value) AS value
FROM data
GROUP BY 1, 2
UNION ALL
SELECT
  interval_id, 'created_frames' AS metric, 'count' AS unit, COUNT(*) AS value
FROM interesting_slice_start
WHERE name = 'ContentRendererClient::RenderFrameCreated'
GROUP BY interval_id;

SELECT * FROM dom_metric;
