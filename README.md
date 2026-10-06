# Steganography
LSB image steganography in C. Hides a secret text file inside a 24-bit BMP image by embedding its bits in the least significant bits of pixel data, and extracts it back with no visible change to the image. Uses a magic string, file extension and size headers. CLI: -e to encode, -d to decode. Modular encode/decode design with error handling.
