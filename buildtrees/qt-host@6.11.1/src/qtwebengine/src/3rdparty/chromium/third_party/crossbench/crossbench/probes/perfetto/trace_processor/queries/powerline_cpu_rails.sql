-- For Google Pixel devices, this query query estimates the power consumed
-- during a PowerLine run using go/pixel-odpm-rails. It includes all rails
-- associated with the SoC compute logic (CPU, GPU, memory etc), but excludes
-- radios, displays etc.

SELECT IMPORT('android.power_rails');

SELECT
  SUM(energy_delta) as total_energy,
  power_rail_name FROM android_power_rails_counters
WHERE power_rail_name LIKE '%CPU%'
  AND power_rail_name NOT LIKE '%CPU%_M%'
GROUP BY power_rail_name
ORDER BY total_energy DESC;