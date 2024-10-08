import unittest
import numpy as np
import q_format

class TestFloatToFixed(unittest.TestCase):
    
    def test_basic_values(self):
        # Simple cases to check if basic functionality is correct
        self.assertEqual(q_format.float_to_fixed(1.5, 5, 7), 192)  # 1.5 in Q5.7 -> 1.5 * 2^7 = 192
        self.assertEqual(q_format.float_to_fixed(0.0, 5, 7), 0)    # 0.0 in any Q format should give 0
        self.assertEqual(q_format.float_to_fixed(-1.5, 5, 7), -192) # -1.5 in Q5.7 -> -1.5 * 2^7 = -192

    def test_edge_cases(self):
        # Test edge cases where values are at the upper and lower bounds
        self.assertEqual(q_format.float_to_fixed(31.999, 5, 7), 4095)  # Max positive value Q5.7
        self.assertEqual(q_format.float_to_fixed(-32.0, 5, 7), -4096)  # Max negative value Q5.7

    def test_overflow(self):
        # Test overflow where the value exceeds what can be represented
        self.assertEqual(q_format.float_to_fixed(33.0, 5, 7), 4095)  # Overflow positive, should saturate
        self.assertEqual(q_format.float_to_fixed(-33.0, 5, 7), -4096)  # Overflow negative, should saturate
    
    def test_rounding(self):
        # Test rounding behavior
        self.assertEqual(q_format.float_to_fixed(0.123, 5, 7), 16)   # 0.123 * 128 ≈ 15.744 -> rounds to 16
        self.assertEqual(q_format.float_to_fixed(0.125, 5, 7), 16)   # 0.125 * 128 = 16 -> no rounding needed
        self.assertEqual(q_format.float_to_fixed(0.124, 5, 7), 16)   # 0.124 * 128 ≈ 15.872 -> rounds to 16
    
    def test_negative_values(self):
        # Test negative values, particularly near overflow boundaries
        self.assertEqual(q_format.float_to_fixed(-31.999, 5, 7), -4096) # Near max negative value Q5.7
        self.assertEqual(q_format.float_to_fixed(-0.5, 5, 7), -64)      # -0.5 in Q5.7 -> -0.5 * 2^7 = -64

if __name__ == '__main__':
    unittest.main()



