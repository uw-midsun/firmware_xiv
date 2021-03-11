#!/usr/bin/env python
"""build script"""
from __future__ import absolute_import, division, print_function, unicode_literals

import os
import sys
import fnmatch
from argparse import ArgumentParser

from mako.lookup import TemplateLookup
from mako.template import Template

import data  # pylint: disable=unused-import

# mostly for importing templates
BASE = os.path.dirname(os.path.abspath(__file__ + '/../').replace('\\', '/'))
sys.path.insert(0, BASE)
PATTERN = '*.mako.*'


def main():
    """The main entry-point of the program"""
    try:
        parser = ArgumentParser()
        add_options(parser)
        options = parser.parse_args()

        if os.path.isfile(options.filename) is False:
            raise IOError(
                'The given file %s could not be found' % options.filename)

        for template in get_templates(BASE, PATTERN):
            code = render(template, __file__=template, options=options)
            write(options.output_dir,
                  os.path.basename(template).replace('mako.', ''), code)
    except Exception as excep:  # pylint: disable=broad-except
        abort('Error: %s' % excep)


def add_options(parser):
    """Add command-line flags

    Args:
        parser: an ArgumentParser

    Returns:
        None
    """
    parser.add_argument(
        '-o',
        '--output',
        dest='output_dir',
        action='store',
        default='out',
        help='output directory')
    parser.add_argument(
        '-f',
        '--filename',
        dest='filename',
        action='store',
        default='can_messages.asciipb',
        help='CAN message asciipb file')


def get_templates(base, pattern):
    """Find templates to render, matching a given pattern

    Args:
        base: the base directory to search for templates
        pattern: a globbing pattern

    Returns:
        a list of matches
    """
    matches = []
    for root, _, filenames in os.walk(base):
        for filename in fnmatch.filter(filenames, pattern):
            matches.append(os.path.join(root, filename))
    return matches


def render(filename, **context):
    """Render a Mako template

    Args:
        filename: the path to the template file to render
        context: the context
    """
    try:
        lookup = TemplateLookup(
            directories=[BASE], input_encoding='utf8', strict_undefined=True)
        template = Template(
            open(filename, 'rb').read(),
            filename=filename,
            input_encoding='utf8',
            lookup=lookup,
            strict_undefined=True)
        # Uncomment to debug generated Python code:
        # write("/tmp", "mako_%s.py" % os.path.basename(filename), template.code)
        return template.render(**context).encode('utf8')
    except Exception as excep:  # pylint: disable=broad-except
        abort('ERROR: %s' % excep)


def abort(message):
    """Abort and display an error message

    Args:
        message: the message to display

    Returns:
        None
    """
    sys.stderr.write(message + "\n")
    sys.exit(1)


def write(directory, filename, content):
    """Write content to filename in directory

    Args:
        directory: the directory to create a file in
        filename: the file to write to
        content: the content to write to the file

    Returns:
        None
    """
    if not os.path.exists(directory):
        os.makedirs(directory)
    with open(os.path.join(directory, filename), 'wb') as outfile:
        outfile.write(content)


if __name__ == "__main__":
    main()
