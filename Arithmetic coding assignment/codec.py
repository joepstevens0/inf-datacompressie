from decimal import Decimal
import decimal
import sys
import json

decimal.getcontext().prec = 200

ALPHABET = [' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z']

def getAlphabetFractions(alphabetInfo):
    # calc total characters used
    total_characters = 0.
    for entry in alphabetInfo:
        total_characters += entry[1]

    # create dictonary for character ranges
    dictionary = {}
    last_top_bound = 0.
    for entry in alphabetInfo:
        range_size = entry[1]/total_characters
        bottom = last_top_bound
        top = bottom + range_size
        dictionary[entry[0]] = {'bottom': Decimal(bottom), 'top': Decimal(top)}
        last_top_bound = top
    return dictionary

def binStringToDecimalFraction(binStr):
    result = Decimal(0.)
    for index, char in enumerate(binStr):
        if char == '0':
            continue
        elif char == '1':
            result +=  Decimal(2)**Decimal(-index - 1)
    return result

def remove_newlines(inputstring: str) -> str:
    result = ""
    for char in inputstring:
        if char == '\n' or char == '\r':
            continue
        result += char
    return result
    
def validate_input(inputstring):
    # Length test
    if (len(inputstring) > 100):
        raise ValueError("Input string exceeded length of 100 characters! Length: "+str(len(inputstring)))

    # Part of alphabet?
    for char in inputstring:
        if not char in ALPHABET:
            raise ValueError("Char '"+str(char)+"' not in alphabet!")

def stringToAlphabetInfo(text):
    letter_amounts = {}
    for char in text:
        if char in letter_amounts:
            letter_amounts[char] += 1
        else:
            letter_amounts[char] = 1
    
    alphabetInfo = []
    for key, val in letter_amounts.items():
        alphabetInfo.append([key, val])
    alphabetInfo.sort()
    return alphabetInfo


def encodeToRange(text):
    try:
        # Validate string
        validate_input(text)
        
        # Create alphabet info
        alphabetInfo = stringToAlphabetInfo(text)

        # get alphabet fractions
        alphabetFractions = getAlphabetFractions(alphabetInfo)

        # Generate bottom and top values
        bottom = Decimal(0.)
        top = Decimal(1.)
        range = Decimal(1.)

        for char in text:
            new_bottom = bottom + range*alphabetFractions[char]['bottom']
            new_top = bottom + range*alphabetFractions[char]['top']
            range = new_top - new_bottom
            bottom = new_bottom
            top = new_top
            

        return (bottom, top, alphabetInfo)
    except Exception as e:
        raise e

def decodeDecimalFraction(avg, alphabetInfo):
    # calc text length
    text_length = 0
    for entry in alphabetInfo:
        text_length += entry[1]

    # get alphabet fractions
    alphabetFractions = getAlphabetFractions(alphabetInfo)

    result = ''
    value = avg
    for i in range(0, text_length):
        bottom = Decimal(0.)
        top = Decimal(1.)
        for key, val in alphabetFractions.items():
            if value >= val['bottom'] and value < val['top']:
                result += key
                bottom = val['bottom']
                top = val['top']
                break
        
        r = top - bottom
        value = (value - bottom)/r
    return result

def test(inputstring):
    try:
        # Encode input
        bottom, top, alphabetInfo = encodeToRange(inputstring)

        # Calc averages
        avg = (bottom + top)/Decimal(2.)

        # Decode encoded string
        decoded = decodeDecimalFraction(avg, alphabetInfo)

        # Compare decoded string to original string
        print("Input string: ", inputstring)
        print("Decoded string: ", decoded)
        if (inputstring == decoded):
            print("Strings are the same!")
        else:
            print("Strings are different.")
            raise "Werkt niet"
    except Exception as e:
        raise e


def generateBinaryStringInRange(bottom, top):
    code = ''
    k = 1
    while binStringToDecimalFraction(code) < bottom:
        
        if binStringToDecimalFraction(code + '1') >= top:
            code += '0'
        else:
            code += '1'
        k = k + 1
        
    return code

def encode(inputstring, outputfile_binstring, outputfile_alphabetinfo):
    # Encode input
    bottom, top, alphabetInfo = encodeToRange(inputstring)

    # Write alphabetInfo to outputfile in JSON format
    try:
        f = open(outputfile_alphabetinfo, "w")
        f.write(json.dumps(alphabetInfo))
        f.close()
    except Exception as e:
        raise e

    # Generate binstring of interval
    binstring = generateBinaryStringInRange(bottom, top)

    # Test binstring in range
    decimal = binStringToDecimalFraction(binstring)
    if not (bottom <= decimal < top):
        raise "Werkt niet"

    # Write binstring to outputfile
    try:
        f = open(outputfile_binstring, "w")
        f.write(binstring)
        f.close()
    except Exception as e:
        raise e


def decode(inputstring, alphabetInfo, outputfile):
    
    decimal = binStringToDecimalFraction(inputstring)
    result = decodeDecimalFraction(decimal, alphabetInfo)

    # Write result to outputfile
    try:
        f = open(outputfile, "w")
        f.write(result)
        f.close()
    except Exception as e:
        raise e

def main():
    try:
        # Check arguments
        arguments = sys.argv
        # print(arguments)

        # filename + mode + inputfilename
        if (len(arguments) < 3):
            raise IndexError("Too few arguments.")

        # Try to open input file
        filename = arguments[2]
        f = open(filename, "r")
        inputstring = f.read()
        inputstring = remove_newlines(inputstring)

        # Check mode
        if (arguments[1] == "test"):
            test(inputstring)
                
        elif (arguments[1] == "encode"):
            # Filename + mode + 3 arguments
            if (len(arguments) >= 5):
                encode(inputstring, arguments[3], arguments[4])
            else:
                raise IndexError("Not enough arguments given for mode 'encode', expected 3.")
        
        elif (arguments[1] == "decode"):
            # Filename + mode + 3 arguments
            if (len(arguments) >= 5):
                # Read alphabetInfo from json
                filename = arguments[3]
                f = open(filename, "r")
                alphabetInfo = f.read()
                alphabetInfo = json.loads(alphabetInfo)

                decode(inputstring, alphabetInfo, arguments[4])
            else:
                raise IndexError("Not enough arguments given for mode 'decode', expected 3.")
        
        else:
            raise ValueError("Mode "+str(arguments[1])+" not known. Try 'test', 'encode' or 'decode'.")
    
    except Exception as e:
        raise e
        
if __name__ == "__main__":
    main()
