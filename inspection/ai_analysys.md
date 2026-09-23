## Group A — Real JDK/CLDC bytecode (works if you source real `.class` files; only needs native fallback otherwise)

**`java.lang.Object`**
- `equals:(Ljava/lang/Object;)Z`
- `getClass:()Ljava/lang/Class;`
- `hashCode:()I` *(not seen directly but implied by Hashtable use)*
- `notify:()V`
- `toString:()Ljava/lang/String;`
- `wait:()V`
- `<init>:()V`

**`java.lang.String`**
- `charAt:(I)C`
- `compareTo:(Ljava/lang/String;)I`
- `endsWith:(Ljava/lang/String;)Z`
- `equals:(Ljava/lang/Object;)Z`
- `getBytes:()[B`
- `indexOf:(I)I`
- `length:()I`
- `substring:(I)Ljava/lang/String;`
- `toLowerCase:()Ljava/lang/String;`
- `valueOf:(I)Ljava/lang/String;`

**`java.lang.StringBuffer`**
- `append:(Ljava/lang/String;)Ljava/lang/StringBuffer;` / `append:(C)Ljava/lang/StringBuffer;` / `append:(I)Ljava/lang/StringBuffer;`
- `delete:(II)Ljava/lang/StringBuffer;` *(seen only as descriptor — flag if not actually called)*
- `setCharAt:(IC)V`
- `toString:()Ljava/lang/String;`

**`java.lang.Integer`**
- `byteValue:()B`
- `intValue:()I`

**`java.lang.Math`**
- `abs:(I)I`
- `abs:(J)J`

**`java.lang.Class`**
- `getResourceAsStream:(Ljava/lang/String;)Ljava/io/InputStream;`

**`java.lang.Thread`**
- `setPriority:(I)V`
- `sleep:(J)V`
- `start:()V`
- `yield:()V`
- `<init>:(Ljava/lang/Runnable;)V`

**`java.io.InputStream`**
- `close:()V`
- `read:()I` / `read:([B)I`
- `skip:(J)J`

**`java.io.ByteArrayInputStream`**
- `<init>:([B)V`

**`java.io.PrintStream`**
- `println:(Ljava/lang/String;)V`

**`java.lang.Throwable`**
- `printStackTrace:()V`

**`java.util.Random`**
- `<init>:(J)V`
- `nextInt:()I`

**`java.util.Hashtable`**
- `get:(Ljava/lang/Object;)Ljava/lang/Object;`
- `put:(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;`

**`javax.microedition.rms.RecordStore`**
- `openRecordStore:(Ljava/lang/String;Z)Ljavax/microedition/rms/RecordStore;`
- `closeRecordStore:()V`
- `addRecord:([BII)I`
- `deleteRecord` *(implied by `delete`)*
- `getNumRecords:()I`
- `getRecord:(I)[B`
- `setRecord:(I[BII)V`

**`javax.microedition.midlet.MIDlet`**
- `getAppProperty:(Ljava/lang/String;)Ljava/lang/String;`
- `notifyDestroyed:()V`
- `notifyPaused:()V`
- `platformRequest:(Ljava/lang/String;)Z`
- `<init>:()V`

---

## Group B — True-native even on a real JVM (always need a native bridge, jar or not)

- `java/lang/System.arraycopy:(Ljava/lang/Object;ILjava/lang/Object;II)V`
- `java/lang/System.currentTimeMillis:()J`
- `java/lang/System.gc:()V`
- Any raw file-open backing `getResourceAsStream` (the *interception point* into your bundled asset files — `/snd.f`, `/ui.f`, `/map_*.out`, etc. seen throughout the dumps — is native regardless of JDK source)

---

## Group C — `lcdui`/`media` (SDL-backed, always native — no real bytecode exists for these even on real hardware)

**`javax.microedition.lcdui.Graphics`**
- `clipRect:(IIII)V`
- `drawImage:(Ljavax/microedition/lcdui/Image;III)V`
- `drawLine:(IIII)V`
- `drawRect:(IIII)V`
- `drawRegion:(...)V` *(9-arg form seen)*
- `drawRoundRect:(IIIIII)V`
- `fillRect:(IIII)V`
- `fillRoundRect:(IIIIII)V`
- `getClipHeight:()I`
- `getClipWidth:()I`
- `getClipX:()I`
- `getClipY:()I`
- `setClip:(IIII)V`
- `setColor:(I)V` / `setColor:(III)V`
- `translate:(II)V`

**`javax.microedition.lcdui.Image`**
- `createImage:(Ljava/lang/String;)Ljavax/microedition/lcdui/Image;` / `createImage:(II)Ljavax/microedition/lcdui/Image;`
- `createRGBImage:([IIIZ)Ljavax/microedition/lcdui/Image;`
- `getGraphics:()Ljavax/microedition/lcdui/Graphics;`
- `getHeight:()I`
- `getWidth:()I`

**`javax.microedition.lcdui.Display`**
- `getDisplay:(Ljavax/microedition/midlet/MIDlet;)Ljavax/microedition/lcdui/Display;`
- `setCurrent:(Ljavax/microedition/lcdui/Displayable;)V`

**`javax.microedition.lcdui.game.GameCanvas`**
- `flushGraphics:()V`
- `setFullScreenMode:(Z)V`
- `getGraphics:()Ljavax/microedition/lcdui/Graphics;`

**`javax.microedition.media.Manager`**
- `createPlayer:(Ljava/io/InputStream;Ljava/lang/String;)Ljavax/microedition/media/Player;`

**`javax.microedition.media.Player`**
- `close:()V`
- `deallocate:()V`
- `getControl:(Ljava/lang/String;)Ljavax/microedition/media/Control;`
- `prefetch:()V`
- `realize:()V`
- `start:()V`
- `addPlayerListener:(Ljavax/microedition/media/PlayerListener;)V`

**`javax.microedition.media.control.VolumeControl`**
- `setLevel:(I)I`

---

## Group D — Game-implemented callbacks (your interpreter *calls into* these via `InvokeVirtual`/`InvokeInterface` — you don't implement them, the game's own bytecode does; just need dispatch, not native bodies)

- `Canvas.keyPressed:(I)V` / `keyReleased:(I)V`
- `Canvas.showNotify:()V` / `hideNotify:()V`
- `MIDlet.startApp:()V` / `pauseApp:()V` / `destroyApp:(Z)V`
- `Runnable.run:()V`
- `PlayerListener.playerUpdate:(Ljavax/microedition/media/Player;Ljava/lang/Object;)V`

These need an SDL event loop (keyboard → `keyPressed`/`keyReleased` calls) and a "run the MIDlet lifecycle" driver (`startApp` on launch, etc.) — not native-method-table entries, but a top-level driver loop you write once `invoke()` is wired to call into game code from the SDL side.

---

**One item flagged separately**: `vibrate` appears in the dump but I can't confirm its exact class/descriptor from what's visible (likely `javax.microedition.lcdui.Display.vibrate:(I)Z`, standard MIDP 2.0 haptics API) — worth confirming against the actual bytecode call site before implementing, since I'm inferring it rather than reading a full signature.

