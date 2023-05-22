# boot2root

## Contents

- [Info](#info)
- [Writeups](#writeups)
    - [Writeup 1](#writeup-1)
        - [lmezard](#lmezard)
        - [laurie](#laurie)
    - [Writeup 2](#writeup-1)
    - [Writeup 3](#writeup-1)
    - [Writeup 4](#writeup-1)
    - [Writeup 5](#writeup-1)


## Info

Project about multiple privilege escalations on Linux.

- Status: finished
- Result: 125%
- Observations:
    - It is compatible with Linux and Mac OS.
    - It does not use `VT100` escape characters.

## Writeups

### Writeup 1

We try to launch the ISO an a VM but it asks for user and password, but the
subject did not provide one.

Let's inspect the ISO squashfs
(squashfs is a compressed read-only file system for Linux):
```bash
tar -xf BornToSecHackMe-v1.1.iso

# 42 mac is shit so I can't install squashfs-tools, so instead I created a
# docker image on the `./scripts` dir
cd ./scripts
docker build -t my-squashfs-tools:latest .
docker run -v /Volumes/PortableSSD/42/casper:/dev/shm -it my-squashfs-tools:latest
```

In the container run `unsquashfs`:
```bash
cd /dev/shm
unsquashfs filesystem.squashfs
```

A lot or errors will popup, ignore them, now let's see what's inside:
```bash
cd squashfs-root/

# Let's add `tree` to easily enumerate dirs
apk add tree
cd home && tree
```

Results:
```raw
.
├── LOOKATME
│   └── password
├── ft_root
│   ├── Desktop
│   └── mail
│       ├── INBOX.Drafts
│       ├── INBOX.Sent
│       └── INBOX.Trash
├── laurie
│   ├── README
│   └── bomb
├── laurie@borntosec.net
│   └── mail
│       ├── INBOX.Drafts
│       ├── INBOX.Sent
│       └── INBOX.Trash
├── lmezard
│   ├── README
│   └── fun
├── thor
│   ├── README
│   └── turtle
└── zaz
    ├── exploit_me
    └── mail
        ├── INBOX.Drafts
        ├── INBOX.Sent
        └── INBOX.Trash
```

Inspecting the ISO on the host fs we found

Inspecting the files we find that:
- `LOOKATME`:
    - Has a password file for `lmezard`, that's our foothold to the machine.
- `ft_root`:
    - Has a `Desktop` dir, but is empty.
    - Note this users is not the real `root`.
- `laurie`:
    - Has a challange that will give us the `thor` user password.
    - The challenge is a binary called `bomb`.
- `lmezard`:
    - Has a challenge that will give us the `laurie` user password.
    - The challenge is a `tar` file full of `pcap`s.
- `thor`:
    - Has a challenge that will give us the `zaz` user password.
    - The challenge is a text file `turtle` that is full of instructions (1471 lines).
- `zaz`:
    - Has a binary `exploit_me`, owned by root.
- All the users have mailboxes, we will check them later.

#### lmezard

Login as `lmezard` with the provided password:
```bash
cat /dev/shm/squashfs-root/home/LOOKATME/password
lmezard:G!@M6f4Eatau{sF"
```

We try to connect to the machine using `ssh` without success, login locally
we check the `/etc/sshd_config` file and found out that `lmezard` is not allowed
to login via `ssh`:
```raw
AllowUsers ft_root zaz thor laurie
```

This is gonna be a headache, I'm a dvorak user and I don't remember
the US layout... Hope `lmezard` challenge is easy and we can login as other
user via `ssh` as soon as possible.

Untar `fun` to other dir where we have perms:
```bash
tar -xf fun -C /dev/shm
cd /dev/shm/ft_fun
```

There are a lot of `pcap` files, let's try `strings`:
```bash
strings * | less
```

We find a lot of `printf("Hahahaha Got you!!!\n");`, and a lot of
`} void useless() {` strings, but other strings got my attention:
```c
char getme10() {
    return 'a';
}
```

It seems to me like there are at least 10 `getme` functions, eachone returning
a character, probably of the `laurie` password, so let's grep for those
functions:
```bash
# The function `getme` seems to have a line for the `return`, in `grep` you can
# use `-B n` to set how many lines print before the match and `-A n` for the
# number of lines after the match, so we can `grep` for `getme` and print one
# line below each time
cat * | grep -A 1 'getme' | less
```

This is what we got:
```raw
//file431char getme4() {

--
//file373char getme7() {

//file736char getme1() {

--
//file668char getme5() {

--
//file722char getme6() {

--
//file138char getme3() {

--
char getme8() {
    return 'w';
--
char getme9() {
    return 'n';
--
char getme10 {
    return 'a';
--
char getme11() {
    return 'g';
--
char getme12
{
--
    printf("%c",getme1());
    printf("%c",getme2());
    printf("%c",getme3());
    printf("%c",getme4());
    printf("%c",getme5());
    printf("%c",getme6());
    printf("%c",getme7());
    printf("%c",getme8());
    printf("%c",getme9());
    printf("%c",getme10());
    printf("%c",getme11());
    printf("%c",getme12());
    printf("\n");
--
//file632char getme2() {
```

We notice that each `pcap` ends with a comment `//fileXX` that indicates
probably the order, so let's make a program to sort them and then displays them
in the correcd order.

The file is in: `./scripts/pcap_sorter.cc`, it will sort and print the files
in the correct order, compile it and run it
(it will expect the directory `./ft_fun`):
```bash
g++ -std=c++17 main.cc
./a.out > source.c
gcc source.c
./a.out
```

And that gives us:
```raw
MY PASSWORD IS: Iheartpwnage
Now SHA-256 it and submit
```

Let's obey and do it:
```bash
echo -n 'Iheartpwnage' | sha256sum
330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4
```

#### laurie

Let's login as `laurie`.
