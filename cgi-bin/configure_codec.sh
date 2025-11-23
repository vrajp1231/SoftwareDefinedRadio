# Reset register (15, 0x00)
i2cset -y 0 0x1A 30 0

# Power down control (6, 0x37)
i2cset -y 0 0x1A 12 0x37

# Left line in (0, 0x80)
i2cset -y 0 0x1A 0 0x80

# Right line in (1, 0x80)
i2cset -y 0 0x1A 2 0x80

# Left headphone out (2, 0x79)
i2cset -y 0 0x1A 4 0x79

# Right headphone out (3, 0x79)
i2cset -y 0 0x1A 6 0x79

# Analog audio path control (4, 0x10)
i2cset -y 0 0x1A 8 0x10

# Digital audio path control (5, 0x00)
i2cset -y 0 0x1A 10 0

# Digital audio interface format (7, 0x02)
i2cset -y 0 0x1A 14 2

# Sampling control (8, 0x00)
i2cset -y 0 0x1A 16 0

# Power down control (6, 0x27) - activate codec
i2cset -y 0 0x1A 12 0x27

# Active control (9, 0x01)
i2cset -y 0 0x1A 18 0x1