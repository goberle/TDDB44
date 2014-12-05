program parstest1;

const
  SIZE1 = 1;

 var
    arr : array[10] of integer;
    i : integer;

function echo(i:integer) : integer;
begin;
    i := 1;
    i := 2;
    return 1;
end;

begin
    i := echo(1,2);
    i := 4?i;
    if (4>i) then
      i := i; 
    end;
    arr[1] := i;
    arr[1] := arr[2];
end.
