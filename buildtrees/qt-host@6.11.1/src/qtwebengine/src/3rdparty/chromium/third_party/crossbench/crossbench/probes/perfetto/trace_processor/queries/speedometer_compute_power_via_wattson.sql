-- For Google Pixel devices, the speedometer_compute_power_via_wattson query
-- estimates the total power consumed during a speedometer run, by leveraging
-- go/wattson-userguide (sorry, Googlers only).

SELECT IMPORT('chrome.speedometer');
SELECT IMPORT('wattson.curves.estimates');

DROP VIEW IF EXISTS speedometer_bounds;
CREATE VIEW speedometer_bounds
AS
SELECT
  MIN(ts) AS ts,
  MAX(ts + dur) - MIN(ts) AS dur
FROM chrome_speedometer_measure;

DROP TABLE IF EXISTS wattson_estimates_in_bounds;
CREATE VIRTUAL TABLE wattson_estimates_in_bounds
USING
  SPAN_JOIN(
    speedometer_bounds,
    _system_state_mw);

DROP VIEW IF EXISTS speedometer_compute_power_via_wattson;
CREATE VIEW speedometer_compute_power_via_wattson
AS
SELECT
  1e-6 * SUM(
    dur * (
      IFNULL(cpu0_mw, 0) + IFNULL(cpu1_mw, 0) + IFNULL(cpu2_mw, 0) +
      IFNULL(cpu3_mw, 0) + IFNULL(cpu4_mw, 0) + IFNULL(cpu5_mw, 0) +
      IFNULL(cpu6_mw, 0) + IFNULL(cpu7_mw, 0) + IFNULL(dsu_scu_mw, 0)))
    AS total_energy_microjoule
FROM wattson_estimates_in_bounds;

SELECT * FROM speedometer_compute_power_via_wattson;
