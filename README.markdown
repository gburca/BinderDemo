This is a bare bones example of how to use Android IPC binders from native C++
code. For some high-level explanations see the following two blog posts:

- http://ebixio.com/blog/2012/07/07/using-android-ipc-binders-from-native-code/
- http://ebixio.com/blog/2011/01/03/the-android-ipc-system/

Quick overview on how to build the code:

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
- The binary should be in `<RepoTop>/out/target/product/*/system/bin/binder`

The code in this project is Copyright(C) by Gabriel Burca and released under the GPL (http://www.gnu.org/copyleft/gpl.html) license.
