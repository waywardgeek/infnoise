#!/usr/bin/env bats

load 'test_helper/bats-support/load'
load 'test_helper/bats-assert/load'

# Tests for the infnoise binary

@test "test --raw output for expected entropy" {
  # capture some data
  TMP_FILE=`mktemp -u $BATS_TMPDIR/infnoise-test-XXXXXXX`
  timeout 5s ./infnoise --raw > $TMP_FILE || true

  # run ent
  run ent $TMP_FILE

  assert_line --index 0 --regexp '^Entropy = 7.2[5-9][0-9]+ bits per byte.$'

  # cleanup
  rm $TMP_FILE
}

@test "test whitened output for expected entropy" {
  # capture some data
  TMP_FILE=`mktemp -u $BATS_TMPDIR/infnoise-test-XXXXXXX`
  timeout 5s ./infnoise > $TMP_FILE || true

  # run ent
  run ent $TMP_FILE

  # check ent's result
  assert_line --index 0 --regexp '^Entropy = 7.99[0-9]+ bits per byte.$'

  # cleanup
  rm $TMP_FILE
}

@test "test whitened output (multiplier=10) for expected entropy" {
  # capture some data
  TMP_FILE=`mktemp -u $BATS_TMPDIR/infnoise-test-XXXXXXX`
  timeout 5s ./infnoise --multiplier 10 > $TMP_FILE || true

  # run ent
  run ent $TMP_FILE

  assert_line --index 0 --regexp '^Entropy = 7.99[0-9]+ bits per byte.$'

  # cleanup
  rm $TMP_FILE
}

@test "test --no-output --debug" {
  # capture some data
  TMP_FILE=`mktemp -u $BATS_TMPDIR/infnoise-test-XXXXXXX`
  run timeout 5s ./infnoise --no-output --debug

  echo $output
  [ "$status" -eq 124 ]

  assert_line --index 0 --regexp '^Generated 1048576 bits.  OK to use data.  Estimated entropy per bit: 0\.[0-8][6-8][0-9]+, estimated K: 1\.8[1-5][0-9]+$'
  assert_line --index 1 --regexp '^num1s:50.[0-9]+%, even misfires:0.[0-1][0-9]+%, odd misfires:0.[0-1][0-9]+%$'
}

@test "test --list-devices" {
  run ./infnoise --list-devices
  echo $output
  [ "$status" -eq 0 ]

  # FTDI serial:
  assert_line --index 1 --regexp '^Manufacturer: FTDI, Description: FT240X USB FIFO, Serial: [0-9A-Z]+$'

  # 13-37.org serial:
  assert_line --index 0 --regexp '^Manufacturer: 13-37.org, Description: Infinite Noise TRNG, Serial: [0-9A-F]+$'
}

@test "test --serial with not connected serial (results in error)" {
  run ./infnoise --serial 4711
  echo $output
  [ "$status" -eq 1 ]
  [ "${lines[0]}" = "Can't find Infinite Noise Multiplier. Try running as super user?" ]
}

@test "test --help" {
  run ./infnoise --help
  echo $output
  [ "$status" -eq 0 ]
  [ "${lines[0]}" = "Usage: infnoise [options]" ]
}
