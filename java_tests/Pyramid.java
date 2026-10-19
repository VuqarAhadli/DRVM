public class Pyramid
{
    public static int createInteger(int row)
    {
        return 2 * row - 1;
    }

    public static void main(String[] args)
    {
        for (int i = 1; i <= 5; i++)
        {
            printSpaces(5 - i);
            printStars(createInteger(i));
            System.out.println("");
        }
    }

    public static void printSpaces(int count)
    {
        for (int i = 0; i < count; i++)
        {
            System.out.print(" ");
        }
    }

    public static void printStars(int count)
    {
        for (int i = 0; i < count; i++)
        {
            System.out.print("*");
        }
    }
}