<p>
        {t escape=no}With the GOsa2 Password Management Add-On you can mass reset user passwords in various ways.{/t}
</p>

{if $pwreset_configured != TRUE}
<input type="hidden" name="phase_01">

<br><h3>{t}Configure password reset options{/t}</h3>

<p>
{t}Please configure options of this run of resetting credentials.{/t}
</p>

<hr>
<table summary="{t}Configure planned password reset operations{/t}">
        <tr><td colspan="3" style="height:0.3em;"></td></tr>
        <tr>
            <td colspan="3">
                    <input {if $preset_pwreset_mode==0} checked {/if} type='radio' name='pwreset_mode' value='0'>{t}Upload a credentials file (CSV format){/t} (no implemented, yet)
                    <br>
            </td>
        </tr>
        <tr><td colspan="3" style="height:0.3em;"></td></tr>
        <tr><td></td><td colspan="2"><b>{t}File format:{/t}</b> {t}CSV, comma-separated, no quotes, two columns:{/t} <tt>&lt;uid&gt;,&lt;userPassword&gt;<tt></td></tr>
        <tr>
                <td style="width: 1em;">&nbsp;</td>
                <td style="vertical-align: top;">
                    <LABEL for="credfile">{t}Select CSV file credential data to upload:{/t}</LABEL>
                </td>
                <td style="vertical-align: middle;">
                        <input type="hidden" name="MAX_FILE_SIZE" value="2097152">
                        <input id="credfile" name="credfile" type="file" value="{t}Browse{/t}">
                <br>
                <div style="padding-top: 0.5em;">(Only needed for credential import via CSV file).</div>
                </td>
        </tr>
        <tr><td colspan="3" style="height:0.8em;"></td></tr>
        <tr>
                <td colspan="3">
                    <input {if $preset_pwreset_mode==1} checked {/if} type='radio' name='pwreset_mode' value='1'>{t}Reset passwords of account in a certain OU{/t} (not implemented, yet)
                    <br>
                </td>
        </tr>
        <tr><td colspan="3" style="height:0.3em;"></td></tr>
        <tr>
                <td style="width: 1em;">&nbsp;</td>
                <td style="vertical-align: middle;">
                        <LABEL for="ou_groups">{t}Select OU for new groups:{/t}</LABEL>
                </td>
                <td style="vertical-align: middle;">
                        <select id="ou_groups" name="ou_groups" size="1" title="">
                        {html_options options=$ous_available selected=$preset_ou_groups}
                        </select>
                </td>
        </tr>

</table>

{elseif $pwreset_reviewed != TRUE}
<input type="hidden" name="phase_02">

<br><h3>{t}Review password resets{/t}</h3>

<p>
{t}Review outstanding password reset operations and deselect individual users.{/t}
</p>

<hr>

<table summary="{t}Review planned password reset operations{/t}">
        <tr>
        <td>FIXME: more work to do...</td>
        </tr>
</table>
{/if}

<hr>
<div class="plugin-actions">

        {if $pwreset_configured != TRUE}
        <input name="btn_pwreset_configured" value="{t}Review upcoming password resets{/t}" type ="submit">
        {elseif $pwreset_reviewed != TRUE}
        <input name="btn_pwreset_reviewed" value="{t}Reset passwords of selected accounts{/t}" type ="submit">
        {/if}
</div>

<input type="hidden" name="ignore">
