#!/usr/bin/env python3
"""New target script.

This module sets up the folder structure and rules.mk for a new project or library.

Usage: python3 new_target.py project|library name
"""
import os
import argparse
import textwrap
from string import Template

RULES_TEMPLATE = Template("""\
    # Defines $$(T)_SRC, $$(T)_INC, $$(T)_DEPS, and $$(T)_CFLAGS for the build makefile.
    # Tests can be excluded by defining $$(T)_EXCLUDE_TESTS.
    # Pre-defined:
    # $$(T)_SRC_ROOT: $$(T)_DIR/src
    # $$(T)_INC_DIRS: $$(T)_DIR/inc{/$$(PLATFORM)}
    # $$(T)_SRC: $$(T)_DIR/src{/$$(PLATFORM)}/*.{c,s}

    # Specify the libraries you want to include
    $$(T)_DEPS := $deps
    """)

README_TEMPLATE = Template("""\
    <!--
        General guidelines
        These are just guidelines, not strict rules - document however seems best.
        A README for a firmware-only project (e.g. Babydriver, MPXE, bootloader, CAN explorer) should answer the following questions:
            - What is it?
            - What problem does it solve?
            - How do I use it? (with usage examples / example commands, etc)
            - How does it work? (architectural overview)
        A README for a board project (powering a hardware board, e.g. power distribution, centre console, charger, BMS carrier) should answer the following questions:
            - What is the purpose of the board?
            - What are all the things that the firmware needs to do?
            - How does it fit into the overall system?
            - How does it work? (architectural overview, e.g. what each module's purpose is or how data flows through the firmware)
    -->
    # $name

    """)


def new_target(target_type, name):
    """Creates a new project or library.

    Creates a subfolder in the appropriate folder with the specified name
    with the following structure:

    projects/libraries
    └── name
        ├── inc
        ├── README.md
        ├── rules.mk
        ├── src
        └── test

    where rules.mk is required for the project or library to be valid.

    Args:
        target_type: Either 'project' or 'library'.
        name: The new project or library's name.

    Returns:
        None
    """
    type_folders = {
        'project': 'projects',
        'library': 'libraries'
    }

    proj_path = os.path.join(type_folders[target_type], name)
    folders = ['src', 'inc', 'test']

    for folder in folders:
        os.makedirs(os.path.join(proj_path, folder), exist_ok=True)

    deps = 'ms-common' if target_type == 'project' else ''

    with open(os.path.join(proj_path, 'rules.mk'), 'w') as rules_file:
        rules_file.write(textwrap.dedent(RULES_TEMPLATE.substitute({'deps': deps})))

    with open(os.path.join(proj_path, 'README.md'), 'w') as readme_file:
        readme_file.write(textwrap.dedent(README_TEMPLATE.substitute({'name': name})))

    print('Created new {0} {1}'.format(target_type, name))


def main():
    """Main entry point of program"""
    parser = argparse.ArgumentParser(description='Creates new project/library')
    parser.add_argument('type', choices=['project', 'library'])
    parser.add_argument('name')
    args = parser.parse_args()

    new_target(args.type, args.name)


if __name__ == '__main__':
    main()
