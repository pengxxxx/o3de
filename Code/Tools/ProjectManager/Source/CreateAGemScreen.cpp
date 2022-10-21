/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <GemCatalog/GemCatalogHeaderWidget.h>
#include <CreateAGemScreen.h>
#include <ProjectUtils.h>
#include <PythonBindingsInterface.h>
#include <ScreenHeaderWidget.h>

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>


namespace O3DE::ProjectManager
{

    CreateGem::CreateGem(QWidget* parent)
        : ScreenWidget(parent)
    {

        QVBoxLayout* screenLayout = new QVBoxLayout();
        screenLayout->setSpacing(0);
        screenLayout->setContentsMargins(0, 0, 0, 0);

        m_header = new ScreenHeader();
        m_header->setSubTitle(tr("Create a new gem"));
        connect(
            m_header->backButton(),
            &QPushButton::clicked,
            this,
            [&]()
            {
                //if previously editing a gem, cancel the operation and revert to old creation workflow
                //this way we prevent accidental stale data for an already existing gem.
                if(m_isEditGem)
                {
                    ClearWorkflow();
                }
                emit GoToPreviousScreenRequest();
            });
        screenLayout->addWidget(m_header);
        
        QHBoxLayout* hLayout = new QHBoxLayout();
        hLayout->setSpacing(0);
        hLayout->setContentsMargins(0, 0, 0, 0);

        QFrame* tabButtonsFrame = CreateTabButtonsFrame();
        hLayout->addWidget(tabButtonsFrame);

        QFrame* tabPaneFrame = CreateTabPaneFrame();
        hLayout->addWidget(tabPaneFrame);
        
        QFrame* createGemFrame = new QFrame();
        createGemFrame->setLayout(hLayout);
        screenLayout->addWidget(createGemFrame);
        
        QFrame* footerFrame = new QFrame();
        footerFrame->setObjectName("createAGemFooter");
        m_backNextButtons = new QDialogButtonBox();
        m_backNextButtons->setObjectName("footer");
        QVBoxLayout* footerLayout = new QVBoxLayout();
        footerLayout->setContentsMargins(0, 0, 0, 0);
        footerFrame->setLayout(footerLayout);
        footerLayout->addWidget(m_backNextButtons);
        screenLayout->addWidget(footerFrame);

        m_backButton = m_backNextButtons->addButton(tr("Back"), QDialogButtonBox::RejectRole);
        m_backButton->setProperty("secondary", true);
        m_nextButton = m_backNextButtons->addButton(tr("Next"), QDialogButtonBox::ApplyRole);

        connect(m_backButton, &QPushButton::clicked, this, &CreateGem::HandleBackButton);
        connect(m_nextButton, &QPushButton::clicked, this, &CreateGem::HandleNextButton);

        setObjectName("createAGemBody");
        setLayout(screenLayout);
    }

    QFrame* CreateGem::CreateTabButtonsFrame()
    {
        QFrame* tabButtonsFrame = new QFrame();
        tabButtonsFrame->setObjectName("createAGemLHS");

        QVBoxLayout* vLayout = new QVBoxLayout();
        vLayout->setSpacing(0);
        vLayout->setContentsMargins(0, 0, 0, 0);

        m_gemTemplateSelectionTab = new QRadioButton(tr("1.  Gem Setup"));
        m_gemDetailsTab =           new QRadioButton(tr("2.  Gem Details"));
        m_gemCreatorDetailsTab =    new QRadioButton(tr("3.  Creator Details"));

        m_gemTemplateSelectionTab->setChecked(true);
        m_gemDetailsTab->setEnabled(false);
        m_gemCreatorDetailsTab->setEnabled(false);

        connect(m_gemTemplateSelectionTab,  &QPushButton::clicked, this, &CreateGem::HandleGemTemplateSelectionTab);
        connect(m_gemDetailsTab,            &QPushButton::clicked, this, &CreateGem::HandleGemDetailsTab);
        connect(m_gemCreatorDetailsTab,     &QPushButton::clicked, this, &CreateGem::HandleGemCreatorDetailsTab);

        vLayout->addSpacing(46);
        vLayout->addWidget(m_gemTemplateSelectionTab);
        vLayout->addSpacing(12);
        vLayout->addWidget(m_gemDetailsTab);
        vLayout->addSpacing(12);
        vLayout->addWidget(m_gemCreatorDetailsTab);
        vLayout->addStretch();

        tabButtonsFrame->setLayout(vLayout);

        return tabButtonsFrame;
    }

    void CreateGem::HandleGemTemplateSelectionTab()
    {
        m_stackWidget->setCurrentIndex(GemTemplateSelectionScreen);
        m_nextButton->setText(tr("Next"));
        m_backButton->setVisible(false);
    }

    void CreateGem::HandleGemDetailsTab()
    {
        m_stackWidget->setCurrentIndex(GemDetailsScreen);
        m_nextButton->setText(tr("Next"));
        m_backButton->setVisible(!m_isEditGem);
    }

    void CreateGem::HandleGemCreatorDetailsTab()
    {
        m_stackWidget->setCurrentIndex(GemCreatorDetailsScreen);
        if(m_isEditGem)
        {
            m_nextButton->setText(tr("Edit"));
        }
        else
        {
            m_nextButton->setText(tr("Create"));
        }
        
        m_backButton->setVisible(true);
    }

    QFrame* CreateGem::CreateTabPaneFrame()
    {
        QFrame* tabPaneFrame = new QFrame();

        QVBoxLayout* vLayout = new QVBoxLayout();
        vLayout->setSpacing(0);
        vLayout->setContentsMargins(0, 0, 0, 0);

        m_stackWidget = new QStackedWidget();
        m_stackWidget->setContentsMargins(0, 0, 0, 0);
        
        m_stackWidget->setObjectName("createAGemRHS");
        m_stackWidget->addWidget(CreateGemSetupScrollArea());
        m_stackWidget->addWidget(CreateGemDetailsScrollArea());
        m_stackWidget->addWidget(CreateGemCreatorScrollArea());
        vLayout->addWidget(m_stackWidget);

        tabPaneFrame->setLayout(vLayout);

        return tabPaneFrame;
    }


    void CreateGem::LoadButtonsFromGemTemplatePaths(QVBoxLayout* gemSetupLayout)
    {
        m_radioButtonGroup = new QButtonGroup();

        auto templatesResult = PythonBindingsInterface::Get()->GetGemTemplates();
        if (templatesResult.IsSuccess() && !templatesResult.GetValue().isEmpty())
        {
            m_gemTemplates = templatesResult.GetValue();
            for (int index = 0; index < m_gemTemplates.size(); ++index)
            {
                const ProjectTemplateInfo& gemTemplate = m_gemTemplates.at(index);

                QRadioButton* button = new QRadioButton(gemTemplate.m_displayName);
                button->setObjectName("createAGem");
                m_radioButtonGroup->addButton(button, index);

                QLabel* buttonSubtext = new QLabel(gemTemplate.m_summary);
                buttonSubtext->setObjectName("createAGemRadioButtonSubtext");

                gemSetupLayout->addWidget(button);
                gemSetupLayout->addWidget(buttonSubtext);
            }
            QAbstractButton* firstButton = m_radioButtonGroup->button(0);
            firstButton->setChecked(true);
        }
    }

    QScrollArea* CreateGem::CreateGemSetupScrollArea()
    {
        QScrollArea* gemSetupScrollArea = new QScrollArea();
        gemSetupScrollArea->setWidgetResizable(true);
        gemSetupScrollArea->setObjectName("createAGemRightPane");
        QFrame* gemSetupFrame = new QFrame();

        QWidget* gemSetupScrollWidget = new QWidget();
        gemSetupScrollArea->setWidget(gemSetupScrollWidget);

        QVBoxLayout* gemSetupLayout = new QVBoxLayout();
        gemSetupLayout->setAlignment(Qt::AlignTop);
        gemSetupLayout->addWidget(gemSetupFrame);
        gemSetupScrollWidget->setLayout(gemSetupLayout);
        QLabel* rightPaneHeader = new QLabel(tr("Please Choose a Gem Template"));
        rightPaneHeader->setObjectName("rightPaneHeader");
        QLabel* rightPaneSubheader = new QLabel(tr("Gems can contain assets new functionality and such as scripts, animations, meshes, textures, and more."));
        rightPaneSubheader->setObjectName("rightPaneSubheader");
        gemSetupLayout->addWidget(rightPaneHeader);
        gemSetupLayout->addWidget(rightPaneSubheader);

        LoadButtonsFromGemTemplatePaths(gemSetupLayout);
        m_formFolderRadioButton = new QRadioButton("Choose existing template");
        m_formFolderRadioButton->setObjectName("createAGem");
        m_radioButtonGroup->addButton(m_formFolderRadioButton);

        m_gemTemplateLocation = new FormFolderBrowseEditWidget(tr("Gem Template Location*"), "", "", tr("A path must be provided."));
        m_gemTemplateLocation->setObjectName("createAGemRadioButtonSubFormField");
        gemSetupLayout->addWidget(m_formFolderRadioButton);
        gemSetupLayout->addWidget(m_gemTemplateLocation);

        return gemSetupScrollArea;
    }

    QScrollArea* CreateGem::CreateGemDetailsScrollArea()
    {
        QScrollArea* gemDetailsScrollArea = new QScrollArea();
        gemDetailsScrollArea->setWidgetResizable(true);
        gemDetailsScrollArea->setObjectName("createAGemRightPane");
        QFrame* gemDetailsFrame = new QFrame();

        QWidget* gemDetailsScrollWidget = new QWidget();
        gemDetailsScrollArea->setWidget(gemDetailsScrollWidget);

        QVBoxLayout* gemDetailsLayout = new QVBoxLayout(this);
        gemDetailsLayout->setAlignment(Qt::AlignTop);
        gemDetailsLayout->addWidget(gemDetailsFrame);
        gemDetailsScrollWidget->setLayout(gemDetailsLayout);

        QLabel* secondRightPaneHeader = new QLabel(tr("Enter Gem Details"));
        secondRightPaneHeader->setObjectName("rightPaneDetailsHeader");
        gemDetailsLayout->addWidget(secondRightPaneHeader);

        m_gemName = new FormLineEditWidget(
            tr("Gem name*"),
            "",
            tr("The unique name for your gem consisting of only alphanumeric characters, '-' and '_'."),
            tr("A gem system name is required."));
        m_gemName->lineEdit()->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-zA-Z]+[a-zA-Z0-9\\-\\_]*"), this));
        gemDetailsLayout->addWidget(m_gemName);

        m_gemDisplayName = new FormLineEditWidget(
            tr("Gem Display name*"), "", tr("The name displayed in the Gem Catalog"), tr("A gem display name is required."));
        m_gemDisplayName->lineEdit()->setValidator(new QRegularExpressionValidator(QRegularExpression("( |\\w)+"), this));
        gemDetailsLayout->addWidget(m_gemDisplayName);

        m_gemSummary = new FormLineEditWidget(tr("Gem Summary"), "", tr("A short description of your Gem"), "");
        gemDetailsLayout->addWidget(m_gemSummary);

        m_requirements = new FormLineEditWidget(tr("Requirements"), "", tr("Notice of any requirements your Gem. i.e. This requires X other gem"), "");
        gemDetailsLayout->addWidget(m_requirements);

        m_license = new FormLineEditWidget(
            tr("License*"), "", tr("License uses goes here: i.e. Apache-2.0 or MIT"), tr("License details are required."));
        gemDetailsLayout->addWidget(m_license);

        m_licenseURL = new FormLineEditWidget(tr("License URL"), "", tr("Link to the license web site i.e. https://opensource.org/licenses/Apache-2.0"), "");
        gemDetailsLayout->addWidget(m_licenseURL);

        m_userDefinedGemTags = new FormLineEditTagsWidget(
            tr("User-defined Gem Tags <i>&nbsp;&nbsp;&nbsp;(Press enter to create Gem Tag)</i>"), "", tr("Tags without spaces: i.e. SampleGame"), "");
        m_userDefinedGemTags->lineEdit()->setValidator(new QRegularExpressionValidator(QRegularExpression("(^$|((\\w+)(\\w*)*))"), this));
        gemDetailsLayout->addWidget(m_userDefinedGemTags);

        m_gemLocation = new FormFolderBrowseEditWidget(tr("Gem Location"), "", tr("The path that the gem will be created at."), tr("The chosen directory must either not exist or be empty."));
        gemDetailsLayout->addWidget(m_gemLocation);
        
        m_gemIconPath = new FormLineEditWidget(tr("Gem Icon Path"), "default.png", tr("Select Gem icon path"), "");
        gemDetailsLayout->addWidget(m_gemIconPath);

        m_documentationURL = new FormLineEditWidget(
            tr("Documentation URL"), "", tr("Link to any documentation of your Gem i.e. https://o3de.org/docs/user-guide/gems/..."), "");
        gemDetailsLayout->addWidget(m_documentationURL);

        return gemDetailsScrollArea;
    }

    QScrollArea* CreateGem::CreateGemCreatorScrollArea()
    {
        QScrollArea* gemCreatorScrollArea = new QScrollArea();
        gemCreatorScrollArea->setWidgetResizable(true);
        gemCreatorScrollArea->setObjectName("createAGemRightPane");
        QFrame* gemCreatorFrame = new QFrame();

        QWidget* gemCreatorScrollWidget = new QWidget();
        gemCreatorScrollArea->setWidget(gemCreatorScrollWidget);

        QVBoxLayout* gemCreatorLayout = new QVBoxLayout();
        gemCreatorLayout->setAlignment(Qt::AlignTop);
        gemCreatorLayout->addWidget(gemCreatorFrame);
        gemCreatorScrollWidget->setLayout(gemCreatorLayout);

        QLabel* thirdRightPaneHeader = new QLabel(tr("Enter your Details"));
        thirdRightPaneHeader->setObjectName("rightPaneDetailsHeader");
        gemCreatorLayout->addWidget(thirdRightPaneHeader);

        m_origin =
            new FormLineEditWidget(tr("Creator Name*"), "", tr("The name of the gem creator or originator goes here. i.e. O3DE"), tr("You must provide a creator name."));
        gemCreatorLayout->addWidget(m_origin);

        m_originURL = new FormLineEditWidget(tr("Origin URL"), "", tr("The primary website for your Gem. i.e. http://o3de.org"), "");
        gemCreatorLayout->addWidget(m_originURL);

        m_repositoryURL = new FormLineEditWidget(tr("Repository URL"), "", tr("Optional URL of the repository for this gem."), "");
        gemCreatorLayout->addWidget(m_repositoryURL);

        return gemCreatorScrollArea;
    }

    bool CreateGem::ValidateGemTemplateLocation()
    {
        bool gemTemplateLocationFilled = true;
        if (m_formFolderRadioButton->isChecked() && m_gemTemplateLocation->lineEdit()->text().isEmpty())
        {
            gemTemplateLocationFilled = false;
        }
        m_gemTemplateLocation->setErrorLabelVisible(!gemTemplateLocationFilled);
        return gemTemplateLocationFilled;
    }

    bool CreateGem::ValidateGemDisplayName()
    {
        bool gemScreenNameIsValid = true;
        if (m_gemDisplayName->lineEdit()->text().isEmpty() || !m_gemDisplayName->lineEdit()->hasAcceptableInput())
        {
            gemScreenNameIsValid = false;
        }
        m_gemDisplayName->setErrorLabelVisible(!gemScreenNameIsValid);
        return gemScreenNameIsValid;
    }

    bool CreateGem::ValidateGemName()
    {
        bool gemSystemNameIsValid = true;
        if (m_gemName->lineEdit()->text().isEmpty() || !m_gemName->lineEdit()->hasAcceptableInput())
        {
            gemSystemNameIsValid = false;
        }
        m_gemName->setErrorLabelVisible(!gemSystemNameIsValid);
        return gemSystemNameIsValid;
    }

    bool CreateGem::ValidateGemPath()
    {
        // This first isEmpty check is to check that the input field is not empty. If it is QDir will use the current
        // directory as the gem location, so that if that folder is also empty it will pass the chosenGemLocation.isEmpty()
        // check and place the created gem there, which is likely unintended behavior for a GUI app such as Project Manager.
        if (m_gemLocation->lineEdit()->text().isEmpty())
        {
            return false;
        }

        QDir chosenGemLocation = QDir(m_gemLocation->lineEdit()->text());
        
        bool locationValid = false;
        
        if(m_isEditGem)
        {
            //If in edit gem workflow, users cannot change this value, so this check should suffice
            locationValid = chosenGemLocation.exists();
        }
        else
        {
            locationValid = !chosenGemLocation.exists() || chosenGemLocation.isEmpty();
        }

        m_gemLocation->setErrorLabelVisible(!locationValid);
        return locationValid;
    }

    bool CreateGem::ValidateFormNotEmpty(FormLineEditWidget* form)
    {
        bool formIsValid = !form->lineEdit()->text().isEmpty();
        form->setErrorLabelVisible(!formIsValid);
        return formIsValid;
    }

    bool CreateGem::ValidateRepositoryURL()
    {
        bool repositoryURLIsValid = true;
        if (!m_repositoryURL->lineEdit()->text().isEmpty())
        {
            QUrl checkURL = QUrl(m_repositoryURL->lineEdit()->text(), QUrl::StrictMode);
            repositoryURLIsValid = checkURL.isValid();
            m_repositoryURL->setErrorLabelText(tr("Repository URL is not valid."));
        }
        m_repositoryURL->setErrorLabelVisible(!repositoryURLIsValid);
        return repositoryURLIsValid;
    }

    void CreateGem::HandleBackButton()
    {
        int indexBackLimit = 0;

        if(m_isEditGem)
        {
            //this will prevent us from reaching the templates page in the edit gem workflow
            indexBackLimit = 1;
        }

        if (m_stackWidget->currentIndex() > indexBackLimit)
        {
            const int newIndex = m_stackWidget->currentIndex() - 1;
            m_stackWidget->setCurrentIndex(newIndex);

            if (newIndex == GemDetailsScreen)
            {
                m_gemDetailsTab->setChecked(true);
            }
            else if (newIndex == GemTemplateSelectionScreen)
            {
                m_gemTemplateSelectionTab->setChecked(true);
            }
        }

        if (m_stackWidget->currentIndex() == indexBackLimit)
        {
            m_backButton->setVisible(false);
        }

        m_nextButton->setText(tr("Next"));
    }

    void CreateGem::HandleNextButton()
    {
        if (m_stackWidget->currentIndex() == GemTemplateSelectionScreen && ValidateGemTemplateLocation())
        {
            m_backButton->setVisible(true);
            m_stackWidget->setCurrentIndex(GemDetailsScreen);
            m_gemDetailsTab->setEnabled(true);
            m_gemDetailsTab->setChecked(true);
        }
        else if (m_stackWidget->currentIndex() == GemDetailsScreen)
        {
            bool gemNameValid = ValidateGemName();
            bool gemDisplayNameValid = ValidateGemDisplayName();
            bool licenseValid = ValidateFormNotEmpty(m_license);
            bool gemPathValid = ValidateGemPath();
            if (gemNameValid && gemDisplayNameValid && licenseValid && gemPathValid)
            {
                m_createGemInfo.m_displayName = m_gemDisplayName->lineEdit()->text();
                m_createGemInfo.m_name = m_gemName->lineEdit()->text();
                m_createGemInfo.m_summary = m_gemSummary->lineEdit()->text();
                m_createGemInfo.m_requirement = m_requirements->lineEdit()->text();
                m_createGemInfo.m_licenseText = m_license->lineEdit()->text();
                m_createGemInfo.m_licenseLink = m_licenseURL->lineEdit()->text();
                m_createGemInfo.m_documentationLink = m_documentationURL->lineEdit()->text();
                m_createGemInfo.m_path = m_gemLocation->lineEdit()->text();
                m_createGemInfo.m_features = m_userDefinedGemTags->getTags();

                m_stackWidget->setCurrentIndex(GemCreatorDetailsScreen);
                if(m_isEditGem)
                {
                    m_nextButton->setText(tr("Edit"));
                    m_backButton->setVisible(true);
                }
                else
                {
                    m_nextButton->setText(tr("Create"));
                }
                
                m_gemCreatorDetailsTab->setEnabled(true);
                m_gemCreatorDetailsTab->setChecked(true);
            }
        }
        else if (m_stackWidget->currentIndex() == GemCreatorDetailsScreen)
        {
            bool originIsValid = ValidateFormNotEmpty(m_origin);
            bool repoURLIsValid = ValidateRepositoryURL();
            if (originIsValid && repoURLIsValid)
            {
                m_createGemInfo.m_origin = m_origin->lineEdit()->text();
                m_createGemInfo.m_originURL = m_originURL->lineEdit()->text();
                m_createGemInfo.m_repoUri = m_repositoryURL->lineEdit()->text();
                QString templateLocation;
                if (m_formFolderRadioButton->isChecked())
                {
                    templateLocation = m_gemTemplateLocation->lineEdit()->text();
                }
                else
                {
                    int templateID = m_radioButtonGroup->checkedId();
                    templateLocation = m_gemTemplates[templateID].m_path;
                }

                if (m_isEditGem)
                {
                    //during editing, we remove the gem name tag to prevent the user accidentally altering it
                    //so add it back here before submission
                    if(!m_createGemInfo.m_features.contains(m_createGemInfo.m_name))
                    {
                        m_createGemInfo.m_features << m_createGemInfo.m_name;
                    }

                    auto result = PythonBindingsInterface::Get()->EditGem(m_oldGemInfo, m_createGemInfo);
                    if(result.IsSuccess())
                    {
                        ClearWorkflow();
                        emit GemEdited(result.GetValue<GemInfo>());
                        emit GoToPreviousScreenRequest();
                    }
                    else
                    {
                        QMessageBox::critical(
                            this,
                            tr("Failed to edit gem"),
                            tr("The gem failed to be edited"));
                    }
                }
                else
                {
                    auto result = PythonBindingsInterface::Get()->CreateGem(templateLocation, m_createGemInfo);
                    if (result.IsSuccess())
                    {
                        ClearWorkflow();
                        emit GemCreated(result.GetValue<GemInfo>());
                        emit GoToPreviousScreenRequest();
                    }
                    else
                    {
                        QMessageBox::critical(
                            this,
                            tr("Failed to create gem"),
                            tr("The gem failed to be created"));
                    }
                }
            }
        }
    }

    //this ignores the templates page
    void CreateGem::ClearWorkflow()
    {
        //details page
        m_gemDisplayName->lineEdit()->clear();
        m_gemDisplayName->setErrorLabelVisible(false);
        
        m_gemName->lineEdit()->clear();
        m_gemName->setErrorLabelVisible(false);
        
        m_gemSummary->lineEdit()->clear();
        m_requirements->lineEdit()->clear();
        
        m_license->lineEdit()->clear();
        m_license->setErrorLabelVisible(false);
        
        m_licenseURL->lineEdit()->clear();
        m_documentationURL->lineEdit()->clear();
        
        m_gemLocation->lineEdit()->clear();
        m_gemLocation->setErrorLabelVisible(false);
        
        m_gemIconPath->lineEdit()->clear();
        m_userDefinedGemTags->clear();

        //creator details page
        m_origin->lineEdit()->clear();
        m_origin->setErrorLabelVisible(false);
        
        m_originURL->lineEdit()->clear();
        m_repositoryURL->lineEdit()->clear();

        //we remove any assumptions about editing a gem, if it was an edit workflow, set it back
        if(m_isEditGem)
        {
            m_isEditGem = false;
            ChangeToCreateWorkflow();
        }
        
    }

    void CreateGem::ChangeToCreateWorkflow()
    {
        m_header->setSubTitle(tr("Create a new gem"));

        //we ensure that we are using all pages
        m_gemTemplateSelectionTab->setVisible(true);
        m_gemTemplateSelectionTab->setChecked(true);

        m_gemDetailsTab->setEnabled(false);
        m_gemDetailsTab->setChecked(false);

        m_gemCreatorDetailsTab->setEnabled(false);
        m_gemCreatorDetailsTab->setChecked(false);

        m_gemDetailsTab->setText(tr("2.  Gem Details"));
        m_gemCreatorDetailsTab->setText(tr("3.  Creator Details"));

        m_stackWidget->setCurrentIndex(GemTemplateSelectionScreen);

        m_gemLocation->setEnabled(true);

        //there exists an edge case where backing out from an old edit gem workflow fails to update next button title
        m_nextButton->setText(tr("Next"));
    }

    void CreateGem::ResetWorkflow(const GemInfo& oldGemInfo, bool isEditWorkflow)
    {
        //details page
        m_gemDisplayName->lineEdit()->setText(oldGemInfo.m_displayName);
        m_gemDisplayName->setErrorLabelVisible(false);

        m_gemName->lineEdit()->setText(oldGemInfo.m_name);
        m_gemName->setErrorLabelVisible(false);

        m_gemSummary->lineEdit()->setText(oldGemInfo.m_summary);
        m_requirements->lineEdit()->setText(oldGemInfo.m_requirement);
        
        m_license->lineEdit()->setText(oldGemInfo.m_licenseText);
        m_license->setErrorLabelVisible(false);

        m_licenseURL->lineEdit()->setText(oldGemInfo.m_licenseLink);
        m_documentationURL->lineEdit()->setText(oldGemInfo.m_documentationLink);
        
        m_gemLocation->lineEdit()->setText(oldGemInfo.m_path);
        m_gemLocation->setErrorLabelVisible(false);

        m_gemIconPath->lineEdit()->setText(oldGemInfo.m_iconPath);

        //before settings the tags field, find a tag that includes the gem name, and remove it
        QStringList tagsToEdit = oldGemInfo.m_features;
        tagsToEdit.removeAll(oldGemInfo.m_name);
        m_userDefinedGemTags->setTags(tagsToEdit);

        //creator details page
        m_origin->lineEdit()->setText(oldGemInfo.m_origin);
        m_origin->setErrorLabelVisible(false);

        m_originURL->lineEdit()->setText(oldGemInfo.m_originURL);
        m_repositoryURL->lineEdit()->setText(oldGemInfo.m_repoUri);

        //mark if we are editing a gem, and if so, change the UI accordingly
        m_isEditGem = isEditWorkflow;
        if(isEditWorkflow)
        {
            ChangeToEditWorkflow();
            m_oldGemInfo = oldGemInfo;
        }
    }

    void CreateGem::ChangeToEditWorkflow()
    {
        m_header->setSubTitle(tr("Edit gem"));

        //we will only have two pages: details page and creator details page
        m_gemTemplateSelectionTab->setChecked(false);
        m_gemTemplateSelectionTab->setVisible(false);

        m_gemDetailsTab->setEnabled(true);
        m_gemDetailsTab->setChecked(true);

        m_gemCreatorDetailsTab->setEnabled(true);

        m_gemDetailsTab->setText(tr("1.  Gem Details"));
        m_gemCreatorDetailsTab->setText(tr("2.  Creator Details"));

        m_stackWidget->setCurrentIndex(GemDetailsScreen);

        m_gemLocation->setEnabled(false);

        //there exists an edge case where backing out from an old create gem workflow fails to update next button title
        m_nextButton->setText(tr("Next"));
    }

} // namespace O3DE::ProjectManager
