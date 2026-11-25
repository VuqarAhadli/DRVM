public class MemoryTest {
    public static void main(String[] args) throws InterruptedException {
        System.out.println("Allocating objects...");

        Object[] objects = new Object[1_000_000];

        for (int i = 0; i < objects.length; i++) {
            objects[i] = new byte[1024];
        }

        System.out.println("Allocated approximately 1 GB.");
        Thread.sleep(10000);

        objects = null;

        System.out.println("Objects released. Requesting GC...");
        System.gc();

        Thread.sleep(10000);
        System.out.println("Finished.");
    }
}
