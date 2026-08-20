# CONTRIBUTING

## How to Contribute

We'd love to accept your patches and contributions to this project. There are
just a few small guidelines you need to follow.

### Contributor License Agreement

Contributions to this project must be accompanied by a Contributor License
Agreement. You (or your employer) retain the copyright to your contribution;
this simply gives us permission to use and redistribute your contributions as
part of the project. Head over to <https://cla.developers.google.com/> to see
your current agreements on file or to sign a new one.

You generally only need to submit a CLA once, so if you've already submitted one
(even if it was for a different project), you probably don't need to do it
again.

### Code Reviews

All submissions, including submissions by project members, require review. We
use Gerrit for this purpose. Consult
[Gerrit Help](https://gerrit-review.googlesource.com/Documentation/intro-gerrit-walkthrough.html)
for more information on using gerrit.

### Community Guidelines

This project follows [Google's Open Source Community
Guidelines](https://opensource.google/conduct/).


## Coding Guidelines
crossbench tries to follow
[Google's Python Style Guide](https://google.github.io/styleguide/pyguide.html).

### General rules
- Avoid code comments, they bit-rot easily. Write tests to document unexpected
  behavior and add helper methods and objects to make your code more readable.
- No code comment that trivially describes what the code does.
- Document classes, tests and config files instead.

### Testing
- New code should be unittested.
- Check local coverage by running
  `poetry run pytest tests/crossbench/ --cov=crossbench --cov-report=html`.
- All ConfigObject classes should have dedicated unittests.

### Typing
- Move all type-checking-only imports in a `if TYPE_CHECKING` block.
- Avoid generic types like `Any`.
- Type-annotate all method parameters and return types.
- Type-annotate variables on first use.

#### Typing-tools
- Fix all mypy errors: `poetry run mypy crossbench --check-untyped-defs `.
- Fix all pytype errors: `poetry run pytype -j auto crossbench`.

### ConfigObject and Input Validation
Any non-trivial configuration should use a dedicate ConfigObject.
We get many benefits from using ConfigObject and ConfigParsers:
- Better warning message on wrong configurations.
- We can easily split complex configurations into separate files.
- ConfigParser can be used to generate help texts.

Coding guidelines:
- Make sure that any configuration is best suited for local running
  and debugging.
- If possible also use dedicated ConfigObject for inner objects.
- Use ConfigParser for parsing serialized configurations in `parse_dict`.
- If possible provide simple shortcuts in the `parse_str`,
  they are useful on the command line.
- Parsing / verifying input values is done using helpers in `crossbench.parse`.
- Failing input validation raises `argparse.ArgumentTypeError`
  so we get proper warning on the cli.
- ConfigObject subclasses have separate unittests.
- Provide example configurations with help texts and explanations in
  `config/doc` and make sure these files are parsed and unittested.


### Maintenance
The supported python versions are listed in the `.vpython3` and the
`pyproject.toml` file.
This has to be in sync with chrome infra's vpython3 version.

- Regularly update the poetry packages using `poetry update`
- Regularly update the vpython3 packages specified in `.vpython3`