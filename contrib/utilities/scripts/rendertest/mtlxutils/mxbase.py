import MaterialX as mx
'''
    Base MaterialX utilities
    - version checking

    Requires: MaterialX package
'''
def haveVersion(major, minor, patch):
    '''
    Check if the current version is at least the given version (current >= given)
    ''' 
    imajor, iminor, ipatch = mx.getVersionIntegers()

    if imajor > major:
        return True
    if imajor == major:
        if iminor > minor:
            return True
        if iminor == minor:
            if ipatch >= patch:
                return True
    return False
