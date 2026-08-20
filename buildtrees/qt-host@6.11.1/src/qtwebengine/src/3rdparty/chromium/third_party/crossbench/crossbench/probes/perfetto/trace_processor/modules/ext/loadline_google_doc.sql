-- LCP
CREATE OR REPLACE PERFETTO FUNCTION loadline_google_doc_score()
RETURNS FLOAT
AS
-- Multiply by 60 to make the score per minutes rather than per second.
SELECT 60e9 / dur
FROM slice
WHERE name = 'PageLoadMetrics.NavigationToLargestContentfulPaint'
ORDER BY ts
LIMIT 1;
