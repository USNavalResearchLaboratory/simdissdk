# Ignore certain files in coverage reports
SET(CTEST_CUSTOM_COVERAGE_EXCLUDE
    ${CTEST_CUSTOM_COVERAGE_EXCLUDE}
    # simVis is not tested in unit tests
    "simVis"
    # ignore moc files that are auto-generated
    "moc_.*"
    # ignore auto-generated protobuf files
    "simData.pb.*"
    # ignore simQt, which cannot be unit tested without a headed display
    "simQt"
)
