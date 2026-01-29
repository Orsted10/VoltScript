// Test JSON functions
print "Testing JSON functions";

// Test jsonEncode
print "jsonEncode(null) = " + jsonEncode(nil);
print "jsonEncode(true) = " + jsonEncode(true);
print "jsonEncode(false) = " + jsonEncode(false);
print "jsonEncode(42) = " + jsonEncode(42);
print "jsonEncode(3.14) = " + jsonEncode(3.14);
print "jsonEncode(\"hello\") = " + jsonEncode("hello");
print "jsonEncode(\"quote\\\"test\") = " + jsonEncode("quote\"test");

// Test jsonDecode
print "jsonDecode(\"null\") = " + str(jsonDecode("null"));
print "jsonDecode(\"true\") = " + str(jsonDecode("true"));
print "jsonDecode(\"false\") = " + str(jsonDecode("false"));
print "jsonDecode(\"42\") = " + str(jsonDecode("42"));
print "jsonDecode(\"3.14\") = " + str(jsonDecode("3.14"));
print "jsonDecode(\"\\\"hello\\\"\") = " + str(jsonDecode("\"hello\""));

print "JSON tests completed";