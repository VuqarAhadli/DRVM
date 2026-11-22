import java.util.Random;

public class RandomNum 
{
    public static void main(String[] args) 
    {

        Random rand = new Random(1234L);
		
        for (int i = 0; i < 15; i++)
        {
            System.out.println(rand.nextInt(99999));
        }
    }
}
