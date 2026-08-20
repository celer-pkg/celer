INCLUDE PERFETTO MODULE ext.loading_interesting_intervals;

DROP VIEW IF EXISTS resource_metric;

CREATE VIEW resource_metric
AS
WITH
  request AS (
    SELECT
      EXTRACT_ARG(arg_set_id, 'debug.data.requestId') AS id,
      EXTRACT_ARG(arg_set_id, 'debug.data.url') AS url,
      ts
    FROM slice
    WHERE name = 'ResourceSendRequest'
  ),
  response AS (
    SELECT
      EXTRACT_ARG(arg_set_id, 'debug.data.requestId') AS id,
      EXTRACT_ARG(arg_set_id, 'debug.data.mimeType') AS mime,
      ts
    FROM slice
    WHERE name = 'ResourceReceiveResponse'
  ),
  finish AS (
    SELECT
      EXTRACT_ARG(arg_set_id, 'debug.data.requestId') AS id,
      EXTRACT_ARG(arg_set_id, 'debug.data.decodedBodyLength') AS len,
      EXTRACT_ARG(arg_set_id, 'debug.data.encodedDataLength') AS data_len,
      ts
    FROM slice
    WHERE name = 'ResourceFinish'
  ),
  resource AS (
    SELECT
      request.ts,
      finish.ts - request.ts AS dur,
      finish.ts - response.ts AS download_dur,
      len,
      CASE
        WHEN url LIKE "data:%" THEN length(url)
        ELSE data_len
      END AS data_len,
      mime,
      CASE
        WHEN mime LIKE '%css%' THEN 'css'
        WHEN mime LIKE '%htm%' THEN 'html'
        WHEN mime LIKE '%html%' THEN 'html'
        WHEN mime LIKE '%json%' THEN 'json'
        WHEN mime LIKE '%font%' THEN 'font'
        WHEN mime LIKE '%audio%' THEN 'audio'
        WHEN mime LIKE '%video%' THEN 'video'
        WHEN mime LIKE '%image%' THEN 'image'
        WHEN mime LIKE 'application/wasm' THEN 'javascript'
        WHEN mime LIKE '%javascript%' THEN 'javascript'
        WHEN mime LIKE '%ecmascript%' THEN 'javascript'
        WHEN mime LIKE '%xml%' THEN 'xml'
        ELSE 'other'
      END AS mime_category,
      CASE
        WHEN length(url) > 100 THEN SUBSTR(url, 1, 90) || '<TRUNCATED>'
        ELSE url
      END AS url
    FROM request, response
    USING (id),
    finish USING (id)
  )
SELECT interval_id, resource.*, resource.ts - navigation_start_ts AS nav_rel_ts
FROM resource, interesting_interval
ON resource.ts BETWEEN interesting_interval.ts AND interesting_interval.end - 1;

SELECT * FROM resource_metric;
