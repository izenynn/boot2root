#!/usr/bin/env bash

# Install sshpass in linux:
# pacman -S sshpass

# Install sshpass on macos:
# brew install hudochenkov/sshpass/sshpass

# Verify if the username is provided as an argument
if [ "$#" -ne 1 ]; then
    echo "Connects to the machine automatically with the user specified" >&2
    echo "Usage: $0 USERNAME" >&2
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
    echo "Usage: $0 USERNAME" >&2
    exit 1
fi

password="${user_line#*:}"
export SSHPASS="${password}"

# Connect
sshpass -e ssh -p 4242 "$user"'@localhost'

exit 0
