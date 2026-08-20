-- For Google Pixel devices, the speedometer_compute_power_via_rails query
-- estimates the total power consumed during a speedometer run, by leveraging
-- go/pixel-odpm-rails counters (sorry, Googlers only). It includes all rails
-- related to the SoC's compute logic (e.g. CPU, GPU, memory, fabric; but
-- excluding e.g. display)

SELECT IMPORT('chrome.speedometer');
SELECT IMPORT('android.power_rails');

-- TODO(b/393047085): Move (some of) these views into TP stdlib to allow reuse.

DROP VIEW IF EXISTS speedometer_bounds;
CREATE VIEW speedometer_bounds AS
SELECT
  MIN(ts) AS ts,
  MAX(ts+dur) - MIN(ts) AS dur
FROM chrome_speedometer_measure;

DROP VIEW IF EXISTS power_rails_for_join;
CREATE VIEW power_rails_for_join AS
SELECT
  *,
  HASH(power_rail_name) as hashed_rail_name
FROM android_power_rails_counters;

DROP TABLE IF EXISTS speedometer_rails_joined;
CREATE VIRTUAL TABLE speedometer_rails_joined
USING SPAN_JOIN(
  speedometer_bounds,
  power_rails_for_join PARTITIONED hashed_rail_name
);

DROP VIEW IF EXISTS speedometer_rail_energies;
CREATE VIEW speedometer_rail_energies AS
SELECT
  *,
  -- consumed energy in microjoule.
  end_energy_uj - start_energy_uj AS energy_delta_uj,
  -- average power in milliwatt.
  (end_energy_uj - start_energy_uj)/dur*1e6 AS average_power_mw
FROM (
  SELECT
    power_rail_name,
    MAX(ts + dur) - MIN(ts) AS dur,
    MIN(energy_since_boot) AS start_energy_uj,
    -- energy_since_boot_at_end may be null if the rail's value didn't change
    -- during Speedometer's run. In that case, end energy = start energy.
    IFNULL(
      MAX(energy_since_boot_at_end),
      MIN(energy_since_boot)) AS end_energy_uj
  FROM speedometer_rails_joined
  GROUP BY power_rail_name
);

-- See documentation at the top of the file.
DROP VIEW IF EXISTS speedometer_compute_power_via_rails;
CREATE VIEW speedometer_compute_power_via_rails AS
SELECT
  SUM(energy_delta_uj) as energy_uj,
  SUM(average_power_mw) as average_power_mw
FROM speedometer_rail_energies
WHERE
  -- TODO(b/393047085): Verify rails to include, this is an informed guess.
  power_rail_name IN (
    'power.rails.cpu.big',
    'power.rails.cpu.mid',
    'power.rails.cpu.little',
    'power.rails.ddr.a',
    'power.rails.ddr.b',
    'power.rails.ddr.c',
    'power.rails.memory.interface',
    'power.rails.system.fabric',
    'power.rails.ldo.main.a',
    'power.rails.ldo.main.b',
    'power.rails.ldo.sub',
    'power.rails.gpu'
  );

SELECT *
FROM speedometer_compute_power_via_rails;
