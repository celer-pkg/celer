CREATE OR REPLACE PERFETTO FUNCTION first_navigation_start()
RETURNS INT
AS
SELECT MIN(ts) AS navigation_start
FROM slice
WHERE name = 'PageLoadMetrics.NavigationToLargestContentfulPaint';

CREATE OR REPLACE PERFETTO FUNCTION last_navigation_start()
RETURNS INT
AS
SELECT MAX(ts) AS last_navigation_start
FROM slice
WHERE name = 'PageLoadMetrics.NavigationToLargestContentfulPaint';
