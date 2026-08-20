#!/usr/bin/env python3
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# /// script
# dependencies = ["check-jsonschema", "click"]
# ///

from __future__ import annotations

from pathlib import Path

import click
from check_jsonschema import main as check_jsonschema_main


SCHEMA_FILES = [
    "modules.json",
]

@click.command(
    context_settings=dict(
        ignore_unknown_options=True,
    )
)
@click.option(
    "--install-prefix",
    type=click.Path(
        exists=True,
        file_okay=False,
        dir_okay=True,
        path_type=Path,
    ),
    required=True,
    metavar="PATH",
    help="Path to the Qt install prefix.",
)
# TODO: Use qtpaths to extract these paths instead
@click.option(
    "--qt-sharedir",
    default="share/qt6",
    metavar="PATH",
    help="The equivalent INSTALL_QT_SHAREDIR that was used.",
)
@click.option(
    "--descriptionsdir",
    default="modules",
    metavar="PATH",
    help="The equivalent INSTALL_DESCRIPTIONSDIR that was used.",
)
@click.argument(
    "check_jsonschema_args",
    nargs=-1,
    type=click.UNPROCESSED,
)
def run(
    install_prefix: Path,
    check_jsonschema_args: list[str],
    qt_sharedir: str,
    descriptionsdir: str,
):
    """
    Validate the module json files after installation.

    Unknown options are passed directly to check-jsonschema.
    """

    # Get the appropriate directory containing the schemas (order is important)
    # If we run the script directly from source, we take the schemas available next to the script
    # to simplify editing the schemas, otherwise we use the installed schema files
    for schemas_dir in [
        Path(__file__).parent,
        install_prefix / f"{qt_sharedir}/json_schema",
    ]:
        # check that all expected schema files are present
        if all((schemas_dir/file).exists() for file in SCHEMA_FILES):
            break
    else:
        click.secho("Error: missing schema files", fg="red")
        raise SystemExit(1)

    module_files = [str(file) for file in (install_prefix / descriptionsdir).glob("*.json")]
    click.echo("Checking modules")
    check_jsonschema_main(
        [
            "--schemafile",
            str(schemas_dir / "modules.json"),
            *check_jsonschema_args,
            *module_files,
        ],
    )


if __name__ == "__main__":
    run()
