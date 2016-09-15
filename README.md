```javascript
var Reader = require('native-readdir')
var fs = require('fs')
var path = require('path')
var assert = require('assert')

// Create a directory.
var directory = '/tmp/example-' + new Date().toISOString()
fs.mkdirSync(directory)

// Write some files in it.
fs.writeFileSync(path.join(directory, 'a'), 'Apple')
fs.writeFileSync(path.join(directory, 'b'), 'Banana')

// Create a readdir object.  Unfortunately, this object encapsulates
// magic:  The POSIX `DIR` pointer from `opendir(3)`.
var reader = new Reader(directory)

// You must call `.open()`.
assert.strictEqual(reader.open(), true)

// Calls `.read()` return strings, then `null`.
assert.strictEqual(reader.read(), '.')
assert.strictEqual(reader.read(), 'a')
assert.strictEqual(reader.read(), 'b')
assert.strictEqual(reader.read(), '..')
assert.strictEqual(reader.read(), null)

// You must call `.close()`.
assert.strictEqual(reader.close(), true)
```
