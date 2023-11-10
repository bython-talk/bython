import lit.formats

config.name = "lit"
config.test_format = lit.formats.ShTest(True)

config.suffixes = [".by"]

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.binary_dir, "test", "lit-output")

config.substitutions.extend([
    (r"%driver-parse", os.path.join(config.binary_dir, r"bython-driver -m=parse")),
    (r"%driver-tcheck", os.path.join(config.binary_dir, r"bython-driver -m=tcheck")),
    (r"%driver-full", os.path.join(config.binary_dir, r"bython-driver -m=full")),
    #(r"%filecheck", "FileCheck-16"),
])