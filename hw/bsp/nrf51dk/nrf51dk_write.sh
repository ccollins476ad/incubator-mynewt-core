#!/bin/bash

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# <binary-path> <flash-offset>

GDB_CMD_FILE=.gdb_cmds

src_file="$1"
dst_off="$2"
src_off="$3"
src_len="$4"

is_int() {
    printf "%d" "$1" > /dev/null 2>&1
}

print_usage() {
    printf 'usage: %s <src-file> <dst-offset> [src-offset] [src-len]\n' "$0" >&2
}

usage_err() {
    if [ ! "$1" = "" ]
    then
        printf '* error: %s\n' "$1" >&2
    fi

    if [ "$2" = "" ]
    then
        rc=1
    else
        rc="$2"
    fi

    print_usage
    exit "$rc"
}

if [ ! -f "$src_file" ]
then
    usage_err $(printf 'file "%s" not found' "$src_file")
fi

if ! is_int "$dst_off"
then
    usage_err $(printf 'invalid dst-offset: %d' "$dst_off")
fi

if [ "$src_off" = '' ]
then
    src_off=0
elif ! is_int "$src_off"
then
    usage_err $(printf 'invalid src-offset: %d' "$src_off")
fi

# If a source length was specified, calculate the source end.  Otherwise, don't
# define a source end; gdb will read the entire file.
if [ ! "$src_len" = '' ]
then
    if ! is_int "$src_len"
    then
        usage_err $(printf 'invalid src-len: %d' "$src_len")
    fi

    src_end=$(($src_off + $src_len))
fi

printf 'Downloading "%s"[%s:%s] to 0x%08x\n' \
    "$src_file" "$src_off" "$src_end" "$dst_off"

# XXX for some reason JLinkExe overwrites flash at offset 0 when
# downloading somewhere in the flash. So need to figure out how to tell it
# not to do that, or report failure if gdb fails to write this file
#
cmds="
shell /bin/sh -c    \
    'trap \"\" 2;   \
     JLinkGDBServer -device nRF51422_xxAC -speed 4000 -if SWD -port 3333 \
        -singlerun' &
target remote localhost:3333
restore $src_file binary $dst_off $src_off $src_end
quit
"

printf '%s' "$cmds" > $GDB_CMD_FILE

msgs=$(arm-none-eabi-gdb -x $GDB_CMD_FILE 2>&1)
printf "%s\n" "$msgs" > .gdb_out

rm $GDB_CMD_FILE

# Echo output from script run, so newt can show it if things go wrong.
printf "%s\n" "$msgs"

if grep 'error' <<< "$msgs"
then
    exit 1
fi

if grep -i 'failed' <<< "$msgs"
then
    exit 1
fi

if grep -i 'unknown / supported' <<< "$msgs"
then
    exit 1
fi

if grep -i 'not found' <<< "$msgs"
then
    exit 1
fi

exit 0
