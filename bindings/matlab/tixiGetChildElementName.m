function value = tixiGetChildElementName(handle, path, index)
    if (ischar(handle))
        error('Invalid handle');
    end
    
    if not(ischar(path))
        error('Invalid xpath');
    end
    
    if (ischar(index))
        error('Invalid index');
    end
    
    try 
        value = tixi_matlab('tixiGetChildElementName',handle, path, index);
    catch  err
        error(err.message);
    end
end