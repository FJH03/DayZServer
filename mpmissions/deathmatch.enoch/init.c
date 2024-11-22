class CustomMission : MissionServer
{
    void CustomMission()
    {
    }

    override void OnRoundStart(DMArena arena)
    {
        // Put your custom round start stuff here
    }

    override void OnRoundEnd(DMArena arena)
    {
        // Put your custom round end stuff here
    }
};

Mission CreateCustomMission(string path)
{
    return new CustomMission();
}

void main()
{
    //INIT ECONOMY--------------------------------------
    Hive ce = CreateHive();
    if (ce)
    {
        ce.InitOffline();
    }

    //DATE RESET AFTER ECONOMY INIT-------------------------
    int year, month, day, hour, minute;
    int reset_month = 8, reset_day = 10;
    GetGame().GetWorld().GetDate(year, month, day, hour, minute);

    if ((month == reset_month) && (day < reset_day))
    {
        GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
    }
    else
    {
        if ((month == reset_month + 1) && (day > reset_day))
        {
            GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
        }
        else
        {
            if ((month < reset_month) || (month > reset_month + 1))
            {
                GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
            }
        }
    }
}
