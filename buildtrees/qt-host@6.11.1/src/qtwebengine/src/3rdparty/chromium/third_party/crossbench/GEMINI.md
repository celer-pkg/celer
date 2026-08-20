# Gemini Workspace Configuration

Always read "./README.md" for all instructions.
Always use `poetry run cb` instead of just running `./cb.py`

Never modify existing crossbench python files.
Never generate python files, only create hjson configs for stories.
Create config.json files for benchmark, story, probe configurations.

Use `poetry run help` to gather all details.
Example config files are in the "config/" folder.

Use `poetry run cb_validate_hjson -- file.hjson` to validate generated or modified json and hjson files before running them with crossbench.
Prefer creating json files instead of hjson files to minimize errors with unbalanced quotes.

Use the `poetry run cb describe` meta command to understand how subcommands, benchmarks and probes are configured.

Use the `--debug` options to get more detailed error message.
Use `--env-validation=warn` to bypass input prompts.

Results are stored in the "results/" folder.
The last run's results are in the "results/latest/last_run" folder.

Run tests with `poetry run pytest tests/crossbench -x -n 7`

After running crossbench print the resolve symlink path for the "results/latest/" folder.