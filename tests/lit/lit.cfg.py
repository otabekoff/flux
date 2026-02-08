import lit.formats

config.name = "Flux"
config.test_format = lit.formats.ShTest(True)
config.suffixes = ['.fl']
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root   = os.path.join(config.test_source_root, '..', '..', 'build', 'tests', 'lit')

# Find the flux compiler binary
import os
build_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'build')
flux_bin = os.path.join(build_dir, 'tools', 'flux', 'flux')
if os.name == 'nt':
    flux_bin += '.exe'

config.substitutions.append(('%flux', flux_bin))
config.substitutions.append(('%FileCheck', 'FileCheck'))
