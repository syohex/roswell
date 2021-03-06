# Roswell - Common Lisp environment setup Utility.

[![Build Status](https://travis-ci.org/snmsts/roswell.svg?branch=master)](https://travis-ci.org/snmsts/roswell)

This software is still beta. Basic interfaces are fixed, we beleive it works well on Unix-like platforms like Linux, Mac OS X and FreeBSD, but some parts are not implemented, not stable on other OSes yet.

## Description
Roswell is a command line tool to install and manage Common Lisp implementations damn easily.

## Limitations
There's a lot of works should be done.

- [SBCL](http://sbcl.org/) is the only one we supports currently ([#9](https://github.com/snmsts/roswell/issues/9))
- Not supports Windows (working on at [#1](https://github.com/snmsts/roswell/issues/1))

## Installation

### Building

#### For Mac OS X users

If you're on Mac OS X and wanna use Homebrew, use the custom tap:

    $ brew tap snmsts/roswell
    $ brew install roswell

#### From source code

    $ git clone -b release https://github.com/snmsts/roswell.git
    $ cd roswell
    $ sh bootstrap
    $ ./configure
    $ make
    $ sudo make install

### Setup

First of all, you have to install Lisp environment. You can install a binary version of SBCL and Quicklisp from internet by this command:

    $ ros setup

After the installation, you can get sub-commands and command line options by this command:

    $ ros

## How to use

### Installing Lisps

If you want to install the newest released version of sbcl you can do it by:

    $ ros install sbcl

and if you have specific version like "1.2.0" to install then

    $ ros install sbcl/1.2.0

will install that version.

You can get which version of SBCL are installed by this command:

    $ ros list installed sbcl

### Setting the default implementation used by ros

    $ ros run -- --version

will show you which version are used. You can change implementation by

    $ ros use sbcl/1.2.3

## Using roswell with Emacs and SLIME

To setup slime with roswell, type like this

```
ros -Q -e '(ql:quickload :quicklisp-slime-helper)' -q
```

Add lines like below to init.el

```lisp
(load (expand-file-name "~/.roswell/impls/ALL/ALL/quicklisp/slime-helper.el"))
(setq inferior-lisp-program "ros -L sbcl -Q run")
```

to load with `.sbclrc`

```lisp
(setq inferior-lisp-program "ros -L sbcl -Q -l ~/.sbclrc run")
```

## Chef recipe for roswell

[@Rudolph-Miller](https://github.com/Rudolph-Miller) is providing a Chef recipe for setting roswell up.

- [Rudolph-Miller/chef-roswell](https://github.com/Rudolph-Miller/chef-roswell)

## Why we named it 'roswell'?
From &lsquo;[made with secret alien technology](http://www.lisperati.com/logo.html)&rsquo;.

I felt making it easier to use Lisp for people is a kind of making opportunity for humanbeings to mingle with alien technology. I recall &lsquo;roswell incident&rsquo; by the concept. I'm not sure what you feel.

## See Also
+ [keens/cim](https://github.com/keens/cim) : similar project that is implemented in shellscript.I motivated to implement roswell by seeing this project.

## Author
SANO Masatoshi (snmsts@gmail.com)

## Contributors
 * Eitaro Fukamachi (e.arrows@gmail.com)
 * Tomoya KAWANISHI (chopsticks.tk.ppfm@gmail.com)

## Special Thanks to
 * Eitaro Fukamachi (e.arrows@gmail.com) for naming this project.

## Project
 * https://github.com/snmsts/roswell

## License
MIT
