INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS interaction_latency_metric;

CREATE VIEW interaction_latency_metric
AS
SELECT
  interval_id,
  "average_interaction_latency" AS metric_name,
  "usec" AS unit,
  avg(original_dur) / 1e3 AS value
FROM interesting_slice_start
WHERE name = 'Responsiveness.Renderer.UserInteraction'
GROUP BY interval_id;

SELECT * FROM interaction_latency_metric;
