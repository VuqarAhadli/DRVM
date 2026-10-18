public class PrintTest {
    public static void main(String[] args) {
        int x = 10;
        int y = 20;
        int z = x + y;
        float k = 6.66f;
        float q = 87245.324f;

        System.out.println(k);   
        System.out.println(q);   

        System.out.println(z);      

        for (int i = 0; i < 8; i++)
        {
            ++x;
            System.out.println(i); 
        }

        System.out.println(x);      
    }
}