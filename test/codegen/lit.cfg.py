import lit.formats

config.name = "codegen"
config.test_format = lit.formats.ShTest(True)

config.suffixes = [".by"]

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.binary_dir, "test", "lit-output")

config.substitutions.append(
    (r"%jit", os.path.join(config.binary_dir, r"bython-jit"))
)