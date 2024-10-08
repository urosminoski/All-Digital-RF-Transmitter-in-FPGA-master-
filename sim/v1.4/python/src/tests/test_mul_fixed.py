import unittest
import numpy as np
import q_format

# Import your q_format.float_to_fixed, q_format.fixed_to_float, and q_format.mul_fixed functions
# Assuming they are in a file named 'fixed_point_ops.py'
# from fixed_point_ops import q_format.float_to_fixed, q_format.fixed_to_float, q_format.mul_fixed

class TestMulFixed(unittest.TestCase):
    
    def test_basic_multiplication(self):
        # Test basic multiplication of two positive numbers
        x_fixed = q_format.float_to_fixed(1.5, 5, 7)  # Q5.7 format
        y_fixed = q_format.float_to_fixed(2.0, 5, 7)  # Q5.7 format
        result = q_format.mul_fixed(x_fixed, y_fixed, 5, 7)
        self.assertAlmostEqual(q_format.fixed_to_float(result, 14), 3.0, places=6)  # Should return 3.0
        
    def test_negative_multiplication(self):
        # Test multiplication of a positive and a negative number
        x_fixed = q_format.float_to_fixed(-1.5, 5, 7)  # Q5.7 format
        y_fixed = q_format.float_to_fixed(2.0, 5, 7)  # Q5.7 format
        result = q_format.mul_fixed(x_fixed, y_fixed, 5, 7)
        self.assertAlmostEqual(q_format.fixed_to_float(result, 14), -3.0, places=6)  # Should return -3.0

    def test_fractional_multiplication(self):
        # Test multiplication of two fractional numbers
        x_fixed = q_format.float_to_fixed(0.25, 5, 7)  # Q5.7 format
        y_fixed = q_format.float_to_fixed(0.5, 5, 7)  # Q5.7 format
        result = q_format.mul_fixed(x_fixed, y_fixed, 5, 7)
        self.assertAlmostEqual(q_format.fixed_to_float(result, 14), 0.125, places=6)  # Should return 0.125

    def test_overflow_multiplication(self):
        # Test for overflow condition
        x_fixed = q_format.float_to_fixed(31.99, 5, 7)  # Near max value for Q5.7
        y_fixed = q_format.float_to_fixed(2.0, 5, 7)    # Q5.7 format
        result = q_format.mul_fixed(x_fixed, y_fixed, 5, 7)
        max_value = 2**(2 * 5 + 2 * 7) - 1  # Max value for Q10.14
        self.assertEqual(result, min(result, max_value))  # Ensure no overflow

    def test_zero_multiplication(self):
        # Test multiplication by zero
        x_fixed = q_format.float_to_fixed(0.0, 5, 7)  # Zero in Q5.7 format
        y_fixed = q_format.float_to_fixed(2.0, 5, 7)  # Q5.7 format
        result = q_format.mul_fixed(x_fixed, y_fixed, 5, 7)
        self.assertEqual(q_format.fixed_to_float(result, 14), 0.0)  # Should return 0.0

    def test_multiplication_by_one(self):
        # Test multiplication by one
        x_fixed = q_format.float_to_fixed(1.0, 5, 7)  # Q5.7 format
        y_fixed = q_format.float_to_fixed(1.0, 5, 7)  # Q5.7 format
        result = q_format.mul_fixed(x_fixed, y_fixed, 5, 7)
        self.assertAlmostEqual(q_format.fixed_to_float(result, 14), 1.0, places=6)  # Should return 1.0

if __name__ == '__main__':
    unittest.main()
