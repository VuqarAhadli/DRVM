import java.util.Random;

public class RandomNum {
    public static void main(String[] args) {

        Random rand = new Random();
		int attempts = 0;
        while(rand.nextInt(99999) != 16)
		{
			attempts++;
		}
		System.out.print("Total attempts java/util/Random.nextInt(I)I:\t");
        System.out.println(attempts);
    }
}
