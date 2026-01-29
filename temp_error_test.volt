// Test error handling
print "Testing error handling";

// Test log with negative number
try {
    print log(-1);
} catch (error) {
    print "Caught error: " + str(error);
}

// Test sin with non-number
try {
    print sin("not a number");
} catch (error) {
    print "Caught error: " + str(error);
}

// Test jsonDecode with non-string
try {
    print jsonDecode(123);
} catch (error) {
    print "Caught error: " + str(error);
}

print "Error handling tests completed";