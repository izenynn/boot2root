#!/usr/bin/env bash

# Install sshpass in linux:
# pacman -S sshpass

# Install sshpass on macos:
# brew install hudochenkov/sshpass/sshpass

# Verify if the username is provided as an argument
if [ "$#" -ne 2 ]; then
    echo "Sends a file to the machine '/dev/shm' directory" >&2
    echo "Usage: $0 USERNAME LOCAL_FILE" >&2
    exit 1
fi

user="$1"

# Verify if the passwords file exists
if [ ! -f passwords.txt ]; then
    echo "File passwords.txt does not exist."
    exit 1
fi

# Search the passwords file for the username
user_line=$(grep "^$user:" passwords.txt)

if [ -z "$user_line" ]; then
    echo "User $user not found in passwords file" >&2
    echo "Usage: $0 USERNAME LOCAL_FILE" >&2
    exit 1
fi

password="${user_line#*:}"
export SSHPASS="${password}"

# Send file
sshpass -e scp -P 4242 "$2" "$user"'@localhost:/dev/shm'

exit 0
