delimiter //

create procedure addrole (accountid bigint, roleidvalue int, rolenamevalue varchar(20), classidvalue int, out addresult int)
begin
	if exists(select * from tb_role where rolename=rolenamevalue) then
	begin
		set addresult = -1;		
	end;
	else
	begin
		insert into tb_role values(accountid, roleidvalue, rolenamevalue,classidvalue);
		set addresult = 0;
	end;		
	end if;
end
//

delimiter ;
