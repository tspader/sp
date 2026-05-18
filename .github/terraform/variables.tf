variable "vouch_app_id" {
  type        = number
  description = "App ID from the Vouch GitHub App's General page."
}

variable "vouch_app_private_key" {
  type        = string
  description = "Private key for the GitHub App installed to the repository"
  sensitive   = true
}
