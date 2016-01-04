This is a bare bones example of how to use Android IPC binders from native C++
code. For some high-level explanations see the following two blog posts:

- http://ebixio.com/blog/2012/07/07/using-android-ipc-binders-from-native-code/
- http://ebixio.com/blog/2011/01/03/the-android-ipc-system/

Quick overview on how to build the code:
----------------------------------------
- This code depends on the parts of Android that are not included in the NDK,
  so you'll need to `repo sync` and build a complete image to have all the
  dependencies in place. For details, see:
    - https://source.android.com/source/downloading.html
- `cd <RepoTop>`
- `source build/envsetup.sh`
- `lunch`
- `cd <RepoTop>/external`
- `git clone https://github.com/gburca/BinderDemo.git`
- `cd BinderDemo`
- `mm`
- The binary should be in `<RepoTop>/out/target/product/*/system/bin/binder`.
  Copy it to /system/bin/ on the phone.

Test the binary:
----------------
- `adb logcat -v time binder_demo:* *:S`
- `adb shell binder`
- `adb shell binder 456`

The `adb logcat -v time binder_demo:* *:S` should show:
```
01-04 12:36:01.453 D/binder_demo( 7575): We're the service
01-04 12:36:01.454 D/binder_demo( 7575): IDemo::IDemo()
01-04 12:36:01.457 D/binder_demo( 7575): Demo service is now ready
01-04 12:36:07.658 D/binder_demo( 7577): We're the client: 456
01-04 12:36:07.660 D/binder_demo( 7577): IDemo::IDemo()
01-04 12:36:07.660 D/binder_demo( 7577): BpDemo::BpDemo()
01-04 12:36:07.660 D/binder_demo( 7577): BpDemo::alert()
01-04 12:36:07.661 D/binder_demo( 7575): BnDemo::onTransact(1) 17
01-04 12:36:07.661 D/binder_demo( 7575): Demo::alert()
01-04 12:36:07.661 D/binder_demo( 7575): BnDemo::onTransact(2) 16
01-04 12:36:07.661 D/binder_demo( 7575): BnDemo::onTransact got 456
01-04 12:36:07.661 D/binder_demo( 7575): Demo::push(456)
01-04 12:36:07.662 D/binder_demo( 7577): BpDemo::push(456)
01-04 12:36:07.662 D/binder_demo( 7575): BnDemo::onTransact(3) 16
01-04 12:36:07.662 D/binder_demo( 7575): Demo::add(456, 5)
01-04 12:36:07.662 D/binder_demo( 7575): BnDemo::onTransact add(456, 5) = 461
01-04 12:36:07.662 D/binder_demo( 7577): BpDemo::add transact reply
01-04 12:36:07.662 D/binder_demo( 7577): BpDemo::add(456, 5) = 461 (status: 0)
01-04 12:36:07.662 D/binder_demo( 7577): Addition result: 456 + 5 = 461
01-04 12:36:07.662 D/binder_demo( 7577): IDemo::~IDemo()
```

The `adb shell binder` should show:
```
Parcel(
  0x00000000: 00400000 00000004 00650044 006f006d '..@.....D.e.m.o.'
  0x00000010: 00000000 00000010 00680054 00200065 '........T.h.e. .'
  0x00000020: 006c0061 00720065 00200074 00740073 'a.l.e.r.t. .s.t.'
  0x00000030: 00690072 0067006e 00000000          'r.i.n.g.....    ')
Demo::alert()
Parcel(
  0x00000000: 00400000 00000004 00650044 006f006d '..@.....D.e.m.o.'
  0x00000010: 00000000 000001c8                   '........        ')
Demo::push(456)
Parcel(NULL)
Parcel(
  0x00000000: 00400000 00000004 00650044 006f006d '..@.....D.e.m.o.'
  0x00000010: 00000000 000001c8 00000005          '............    ')
Demo::add(456, 5)
Parcel(NULL)
```

The `adb shell binder 456` should show:
```
We're the client: 456
BpDemo::push parcel to be sent:
Parcel(
  0x00000000: 00400000 00000004 00650044 006f006d '..@.....D.e.m.o.'
  0x00000010: 00000000 000001c8                   '........        ')
BpDemo::push parcel reply:
Parcel(NULL)
BpDemo::add parcel to be sent:
Parcel(
  0x00000000: 00400000 00000004 00650044 006f006d '..@.....D.e.m.o.'
  0x00000010: 00000000 000001c8 00000005          '............    ')
Parcel(000001cd    '....')
```

The server added 5 to our 456 and returned 461 (0x01cd).

The code in this project is Copyright(C) by Gabriel Burca and released under the GPL (http://www.gnu.org/copyleft/gpl.html) license.
