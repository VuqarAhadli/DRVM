public class Test
{
    static int staticValue = 7;

    int value;

    Test(int value)
    {
        this.value = value;
    }

    int add(int x)
    {
        return value + x;
    }

    int multiply(int x, int y)
    {
        return x * y;
    }

    static int staticAdd(int a, int b)
    {
        return a + b;
    }

    static int factorial(int n)
    {
        if (n <= 1)
        {
            return 1;
        }

        return n * factorial(n - 1);
    }

    static int fibonacci(int n)
    {
        if (n <= 1)
        {
            return n;
        }

        return fibonacci(n - 1) + fibonacci(n - 2);
    }

    static int switchDense(int x)
    {
        switch (x)
        {
            case 0:
                return 10;
            case 1:
                return 20;
            case 2:
                return 30;
            case 3:
                return 40;
            case 4:
                return 50;
            default:
                return -1;
        }
    }

    static int switchSparse(int x)
    {
        switch (x)
        {
            case -100:
                return 1;
            case 7:
                return 2;
            case 42:
                return 3;
            case 999:
                return 4;
            default:
                return -1;
        }
    }

    static int arrayTest()
    {
        int[] values = new int[10];

        for (int i = 0; i < values.length; ++i)
        {
            values[i] = i * i;
        }

        int sum = 0;

        for (int i = 0; i < values.length; ++i)
        {
            sum += values[i];
        }

        return sum;
    }

    static int multiArrayTest()
    {
        int[][] matrix = new int[3][4];

        int value = 1;

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                matrix[i][j] = value++;
            }
        }

        return matrix[0][0]
            + matrix[1][1]
            + matrix[2][2];
    }

    static int objectArrayTest()
    {
        Test[] objects = new Test[3];

        objects[0] = new Test(10);
        objects[1] = new Test(20);
        objects[2] = new Test(30);

        int sum = 0;

        for (int i = 0; i < objects.length; ++i)
        {
            sum += objects[i].value;
        }

        return sum;
    }

    static int typeTest()
    {
        Object object = new Test(123);

        if (object instanceof Test)
        {
            Test test = (Test)object;
            return test.value;
        }

        return -1;
    }

    static long longTest()
    {
        long a = 123456789L;
        long b = 987654321L;

        long result = a + b;
        result ^= 0x55AA55AAL;
        result <<= 3;
        result >>= 1;

        return result;
    }

    static float floatTest()
    {
        float a = 3.5f;
        float b = 2.25f;

        return (a * b) + 1.5f;
    }

    static double doubleTest()
    {
        double a = 10.5;
        double b = 2.5;

        return (a / b) + 7.25;
    }

    static int bitTest()
    {
        int a = 0x55;
        int b = 0x0F;

        int result = a & b;
        result |= 0x20;
        result ^= 0x03;
        result <<= 2;
        result >>= 1;

        return result;
    }

    static int comparisonTest()
    {
        int a = 10;
        int b = 20;
        int result = 0;

        if (a < b)
        {
            result += 1;
        }

        if (a <= b)
        {
            result += 2;
        }

        if (b > a)
        {
            result += 4;
        }

        if (b >= a)
        {
            result += 8;
        }

        if (a != b)
        {
            result += 16;
        }

        if (a == 10)
        {
            result += 32;
        }

        return result;
    }

    static int booleanTest()
    {
        boolean a = true;
        boolean b = false;

        int result = 0;

        if (a)
        {
            result += 10;
        }

        if (!b)
        {
            result += 20;
        }

        return result;
    }

    static int charShortByteTest()
    {
        char c = 'A';
        short s = 1234;
        byte b = 12;

        return c + s + b;
    }

    static int nestedLoopTest()
    {
        int result = 0;

        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 5; ++j)
            {
                result += i * j;
            }
        }

        return result;
    }

    static int virtualDispatchTest()
    {
        Test a = new Test(10);
        Test b = new Test(20);

        return a.add(5) + b.add(5);
    }

    static int interfaceTest()
    {
        Calculator calculator = new CalculatorImpl();

        return calculator.calculate(12, 8);
    }

    static int staticFieldTest()
    {
        return staticValue;
    }

    static int incrementTest()
    {
        int x = 0;

        ++x;
        x++;
        --x;
        x--;

        x += 10;
        x -= 3;
        x *= 2;
        x /= 7;
        x %= 5;

        return x;
    }

    public static void main(String[] args)
    {
        int total = 0;

        total += staticAdd(10, 20);
        total += factorial(6);
        total += fibonacci(10);

        total += switchDense(3);
        total += switchSparse(42);

        total += arrayTest();
        total += multiArrayTest();
        total += objectArrayTest();

        total += typeTest();

        total += (int)longTest();
        total += (int)floatTest();
        total += (int)doubleTest();

        total += bitTest();
        total += comparisonTest();
        total += booleanTest();
        total += charShortByteTest();

        total += nestedLoopTest();
        total += virtualDispatchTest();
        total += interfaceTest();

        total += staticFieldTest();
        total += incrementTest();

        Test object = new Test(100);

        total += object.add(50);
        total += object.multiply(6, 7);

        int result = total;

        if (result == 1581354567)
        {
            return;
        }

        throw new RuntimeException();
    }

    interface Calculator
    {
        int calculate(int a, int b);
    }

    static class CalculatorImpl implements Calculator
    {
        public int calculate(int a, int b)
        {
            return (a * 2) + (b * 3);
        }
    }

}
