import re
import sys


def xor_encode(hexcode, key):
    """
    XOR encodes the given hexcode with the provided key.

    Args:
        hexcode (bytes): The input bytes of the hexcode.
        key (bytes): The key used to XOR encode the hexcode.

    Returns:
        bytes: The XOR-encoded bytes.
    """
    encoded_bytes = bytearray()
    for i, byte in enumerate(hexcode):
        encoded_bytes.append(byte ^ key[i % len(key)])
    return encoded_bytes


def parse_hexcode(input_data):
    """
    Parses the hexcode in the format "0xXX, 0xYY, ..." into raw bytes.

    Args:
        input_data (str): The input hexcode in the specified format.

    Returns:
        bytes: A bytes object containing the parsed hex values.
    """
    # Extract all "0xXX" hex values using regex
    matches = re.findall(r'0x([0-9a-fA-F]{2})', input_data)
    return bytes(int(match, 16) for match in matches)


def format_as_hexcode(encoded_bytes):
    """
    Formats the output bytes into the input format "0xXX, 0xYY, ...".

    Args:
        encoded_bytes (bytes): The XOR-encoded bytes.

    Returns:
        str: The formatted hexcode string.
    """
    formatted = ", ".join(f"0x{byte:02x}" for byte in encoded_bytes)
    # Group into lines of 8 bytes for readability
    return "\n  ".join([formatted[i:i+48] for i in range(0, len(formatted), 48)])


if __name__ == "__main__":
    # Check if the user provided the file path and key as arguments
    if len(sys.argv) != 3:
        print("Usage: python xor_encoder_file_input.py <file_path> <key>")
        print("Example key format: 0x67,0x89,0xab")
        sys.exit(1)

    file_path = sys.argv[1]
    key_input = sys.argv[2]

    try:
        # Read the hexcode from the file
        with open(file_path, "r") as file:
            hexcode_data = file.read()

        # Parse the hexcode into raw bytes
        hexcode = parse_hexcode(hexcode_data)

        # Parse the XOR key into raw bytes
        key = parse_hexcode(key_input)

        # Perform XOR encoding
        encoded_bytes = xor_encode(hexcode, key)

        # Format the encoded bytes as the input format
        encoded_hexcode = format_as_hexcode(encoded_bytes)
        print(f"{encoded_hexcode}")

    except FileNotFoundError:
        print(f"File not found: {file_path}")
        sys.exit(1)

    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)
