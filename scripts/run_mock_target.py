# PlatformIO extra_script for the native envs. Adds a "mock" target that
# builds the env's program, runs it against the default mock PIR script
# (lib/mock-arduino/script.txt — override with AUNISOMA_MOCK_SCRIPT), and
# writes the JSON output to script.json for the mock HTML page.
#
#   pio run -e native_glow -t mock
Import("env")

env.AddCustomTarget(
    name="mock",
    dependencies="$BUILD_DIR/${PROGNAME}${PROGSUFFIX}",
    actions=['"$BUILD_DIR/${PROGNAME}${PROGSUFFIX}" > "$PROJECT_DIR/script.json"'],
    title="Run mock",
    description="Run the native binary against lib/mock-arduino/script.txt and write script.json",
)
