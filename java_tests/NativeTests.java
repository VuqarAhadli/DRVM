/**
 *  Exercises every native method registered so far:
 *    System.gc, Math.abs (I, J, F, D),
 *    PrintStream.print / println (I, J, F, D, Z, C, String) and println().
 *
 *  No string concatenation is used on purpose: javac turns "a" + b into
 *  StringBuffer/StringBuilder calls, which the VM doesn't implement yet.
 *  Every line below prints via a single print/println call.
 */
public class NativeTests
{
    public static void main(String[] args)
    {
        section("int");
        System.out.println(0);
        System.out.println(55);
        System.out.println(-55);
        System.out.println(2147483647);
        System.out.println(-2147483648);
        byte b = -5;
        short s = 1234;
        System.out.println(b);          
        System.out.println(s);

        section("long");
        System.out.println(0L);
        System.out.println(9876543210L);
        System.out.println(-9876543210L);
        System.out.println(9223372036854775807L);
        System.out.println(-9223372036854775808L);
        System.out.print(123L);
        System.out.println();

        section("float");
        System.out.println(1.0f);
        System.out.println(0.5f);
        System.out.println(-123.25f);
        System.out.println(100.0f);
        System.out.println(0.001f);
        System.out.println(1.0E10f);
        System.out.println(1.5E-5f);
        float fz = 0.0f;
        System.out.println(fz);
        System.out.println(-1 * fz);        
        System.out.println(fz / fz);   
        System.out.println(1.0f / fz);  
        System.out.println(-1.0f / fz);

        section("double");
        System.out.println(1.0);
        System.out.println(0.5);
        System.out.println(3.14159);
        System.out.println(-123.45);
        System.out.println(12345.678);
        System.out.println(9999999.0);  // largest decimal range
        System.out.println(10000000.0); // switches to 1.0E7
        System.out.println(0.001);      // smallest decimal range
        System.out.println(0.0001);     // switches to 1.0E-4
        System.out.println(1.234E-5);
        System.out.println(1.0E100);
        double dz = 0.0;
        System.out.println(dz);
        System.out.println(-dz);        
        System.out.println(dz / dz);    
        System.out.println(1.0 / dz);   
        System.out.println(-1.0 / dz);  

        section("boolean");
        System.out.println(true);
        System.out.println(false);
        int five = 5;
        System.out.println(five > 3);
        System.out.println(five < 3);

        section("char");
        System.out.println('A');
        System.out.println('z');
        System.out.println('7');
        System.out.println('\u00e9');   
        System.out.println('\u20ac');   
        System.out.print('H');
        System.out.print('i');
        System.out.println();

        section("String");
        System.out.println("Hello, VM!");
        System.out.println("");
        System.out.println("tab\there");
        System.out.println("caf\u00e9 \u20ac");
        System.out.print("no newline, ");
        System.out.println("then newline");

        section("print / println mix");
        System.out.print(1);
        System.out.print(' ');
        System.out.print(2L);
        System.out.print(' ');
        System.out.print(3.0f);
        System.out.print(' ');
        System.out.print(4.0);
        System.out.print(' ');
        System.out.print(true);
        System.out.print(' ');
        System.out.print("five");
        System.out.println();

        section("Math.abs int");
        int i1 = -7;
        int i2 = 7;
        int i3 = 0;
        int iMin = -2147483648;
        System.out.println(Math.abs(i1));     
        System.out.println(Math.abs(i2));     
        System.out.println(Math.abs(i3));     
        System.out.println(Math.abs(iMin));   // -2147483648 (Java quirk)

        section("Math.abs long");
        long l1 = -7L;
        long l2 = 9223372036854775807L;
        long lMin = -9223372036854775808L;
        System.out.println(Math.abs(l1));     
        System.out.println(Math.abs(l2));     
        System.out.println(Math.abs(lMin));   // -9223372036854775808 (Java quirk)

        section("Math.abs float");
        float f1 = -2.5f;
        float f2 = 2.5f;
        float fnz = -0.0f;
        System.out.println(Math.abs(f1));     
        System.out.println(Math.abs(f2));     
        System.out.println(Math.abs(fnz));    
        System.out.println(Math.abs(fz / fz));// NaN

        section("Math.abs double");
        double d1 = -2.5;
        double d2 = 2.5;
        double dnz = -0.0;
        System.out.println(Math.abs(d1));     
        System.out.println(Math.abs(d2));  
        System.out.println(Math.abs(dnz));  
        System.out.println(Math.abs(-1.0 / dz)); // Infinity

        section("System.gc");
        System.gc();
        System.gc();
        System.out.println("gc survived");

        section("done");
    }

    private static void section(String name)
    {
        System.out.println();
        System.out.print("<=- ");
        System.out.print(name);
        System.out.println(" -=>");
    }
}