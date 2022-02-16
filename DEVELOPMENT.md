# MaterialX Development Environment
## Autodesk Fork 
- https://github.com/materialx/MaterialX
- This is where Autodesk specific additions to ASWF main fork reside.
- Any areas of core which are not part of main should be resolved. ADSK core and ASWF core should always be in sync per release.
- Additions include things like additional code generators, utilities etc.
- Branches:
    - The main branch should be kept up to date with ASWF main. Nothing ever gets merged back to this main branch.
    - The `adsk_contrib/dev` : is the main working branch
        - Tagged released corresponding to ASWF releases are based on this branch.
    - If and only if needed a new release branch can be created. Thus far this has never been done.

## Status / Releases
- 	The general wiki is here (https://github.com/autodesk-forks/MaterialX/wiki). Please keep this up to date.
- 	Every release of ASWF should have a corresponding ADSK release. The ASWF tag names are v<major>.<minor><patch>. The ASDK tag names include a prefix of _adsk.
    - Note that distributions are still manually added back in though possibly an action could be added for this.
    - E.g.  
     ![image](https://user-images.githubusercontent.com/14275104/154177532-06a9d983-f7a8-4e14-a471-57618bf4f722.png)

## Contributing
### Issues
- Before starting work, if non-trivial should be checked with ASWF gatekeepers. The ASWF #materialx Slack channel is a good place to start. This is for both implementations and required for specification changes.
- An issue can be raised either on the ASWF repo if a common item. 
  ![image](https://user-images.githubusercontent.com/14275104/154177600-4395e218-4027-4ea7-81fc-89de9ad9d7f5.png)
- If ADSK specific an issue can be added in the ADSK fork issue section.
  ![image](https://user-images.githubusercontent.com/14275104/154177637-18bdc393-c586-4991-a13a-d8f1927e3aeb.png)

    - Please add labels as appropriate, release number should be checked with the ADSK owner, and assignments added as needed. The ASWF repo has no labels at time of writing.
    - There is a [project Kanban board](https://github.com/autodesk-forks/MaterialX/projects) which is automated.
      ![image](https://user-images.githubusercontent.com/14275104/154177691-1fe4d9f9-8d81-48cb-ae02-448a75c320a5.png)
 

### Coding and Pull Requests
- 	All work in progress should have branch off of `adsk_contrib/dev` with a name `adsk_contrib/<name of work>` if origin is from ADSK.
- 	All work if possible, should be relative to ASWF main. The easiest thing to do is update ADSK `main` and create a branch off of it.
- 	PR’s when ready should be directly against ASWF main though it can be committed to the ADSK branch first for CI/CD testing
    ![image](https://user-images.githubusercontent.com/14275104/154177799-45dec73c-e846-432c-8c74-9cbfe9b4ad48.png)

- 	All PRs should 
o	Reference a github issue and/or ADSK JIRA issue if  applicable.
o	Include at least one reviewer. ASWF members can be added as necessary.
o	Should have a release tag for tracking.
o	The CHANGELOG.md should be updated as needed.

## CI/CD and other Automated Processes
- ADSK has a slight variation on Github actions in that not all configurations are configuration for a push commit.
- A PR will trigger the exact same pipeline as the ASWF one.
  - PRs are hooked up to Codacy for static analysis.
  - There is only one workflow file (main.yml) which handles desktop and web CI.  
   ![image](https://user-images.githubusercontent.com/14275104/154177845-734416f2-b646-4e74-8ccf-ca108ada5c50.png)
- The web CI will update the associated github page on PRs.
   ![image](https://user-images.githubusercontent.com/14275104/154177869-ccd30370-4a0c-41da-a6c2-cdbcf84db753.png)
- `dependabot` is used to check and automatically create PRs for package dependency security issues. Admins can control the configuration of this. 
  ![image](https://user-images.githubusercontent.com/14275104/154177897-62e7b02b-1233-43de-8f2d-6e270b203c6d.png)


